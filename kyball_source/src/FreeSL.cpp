#include "FreeSL.h"

#include <AL/al.h>
#include <AL/alc.h>

#include <zlib.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <fstream>
#include <map>
#include <mutex>
#include <string>
#include <vector>

extern "C" int stb_vorbis_decode_memory(const unsigned char* mem, int len, int* channels, int* sample_rate, short** output);

namespace FreeSL {
namespace {

	struct Voice {
		ALuint source;
		bool streaming;
		bool alive;
		float baseGain;
		float gainMul;
		float basePitch;
		float pitchMul;
	};

	ALCdevice* device = 0;
	ALCcontext* context = 0;
	bool ready = false;
	bool silent = false;
	float masterVolume = 1.f;
	// SoundBoss holds this lock and then calls fsl* functions that lock again.
	// The original FreeSL critical section was recursive; std::mutex deadlocks
	// there, so Map::init never returns and the title frame stays on screen.
	std::recursive_mutex mutex;
	std::map<unsigned int, Voice> voices;
	std::map<unsigned int, bool> buffers;
	unsigned int nextFake = 1;

	void applyGain(Voice& voice) {
		if (silent || !voice.source) return;
		alSourcef(voice.source, AL_GAIN, voice.baseGain * voice.gainMul);
	}

	void applyPitch(Voice& voice) {
		if (silent || !voice.source) return;
		float pitch = voice.basePitch * voice.pitchMul;
		if (pitch < 0.01f) pitch = 0.01f;
		alSourcef(voice.source, AL_PITCH, pitch);
	}

	bool readFile(const char* path, std::vector<unsigned char>& out) {
		std::ifstream in(path, std::ios::binary);
		if (!in) return false;
		in.seekg(0, std::ios::end);
		std::streamoff n = in.tellg();
		if (n <= 0) return false;
		out.resize((size_t)n);
		in.seekg(0, std::ios::beg);
		in.read(reinterpret_cast<char*>(&out[0]), n);
		return (bool)in || in.eof();
	}

	uint16_t ru16(const unsigned char* p) {
		return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
	}
	uint32_t ru32(const unsigned char* p) {
		return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
	}

	bool namesMatch(const std::string& entry, const char* wanted) {
		if (!wanted) return false;
		if (entry == wanted) return true;
		std::string a = entry, b = wanted;
		for (size_t i = 0; i < a.size(); ++i) if (a[i] == '\\') a[i] = '/';
		for (size_t i = 0; i < b.size(); ++i) if (b[i] == '\\') b[i] = '/';
		return a == b;
	}

	bool unzipEntry(const std::vector<unsigned char>& zip, const char* name, std::vector<unsigned char>& out) {
		if (zip.size() < 22 || !name) return false;
		size_t start = zip.size() > 66000 ? zip.size() - 66000 : 0;
		size_t eocd = (size_t)-1;
		for (size_t i = zip.size() - 22; ; --i) {
			if (i < start) break;
			if (ru32(&zip[i]) == 0x06054b50u) { eocd = i; break; }
			if (i == 0) break;
		}
		if (eocd == (size_t)-1 || eocd + 22 > zip.size()) return false;
		uint16_t entries = ru16(&zip[eocd + 10]);
		uint32_t cdOff = ru32(&zip[eocd + 16]);
		size_t p = cdOff;
		for (uint16_t n = 0; n < entries; ++n) {
			if (p + 46 > zip.size() || ru32(&zip[p]) != 0x02014b50u) return false;
			uint16_t method = ru16(&zip[p + 10]);
			uint32_t compSize = ru32(&zip[p + 20]);
			uint32_t uncompSize = ru32(&zip[p + 24]);
			uint16_t nameLen = ru16(&zip[p + 28]);
			uint16_t extraLen = ru16(&zip[p + 30]);
			uint16_t commentLen = ru16(&zip[p + 32]);
			uint32_t localOff = ru32(&zip[p + 42]);
			if (p + 46 + nameLen > zip.size()) return false;
			std::string entry(reinterpret_cast<const char*>(&zip[p + 46]), nameLen);
			p += 46 + nameLen + extraLen + commentLen;
			if (!namesMatch(entry, name)) continue;
			if ((size_t)localOff + 30 > zip.size()) return false;
			uint16_t localName = ru16(&zip[localOff + 26]);
			uint16_t localExtra = ru16(&zip[localOff + 28]);
			size_t dataOff = (size_t)localOff + 30 + localName + localExtra;
			if (dataOff + compSize > zip.size()) return false;
			if (method == 0) {
				out.assign(zip.begin() + dataOff, zip.begin() + dataOff + compSize);
				return true;
			}
			if (method == 8) {
				out.resize(uncompSize ? uncompSize : 1);
				z_stream strm;
				memset(&strm, 0, sizeof(strm));
				strm.next_in = const_cast<Bytef*>(&zip[dataOff]);
				strm.avail_in = compSize;
				strm.next_out = &out[0];
				strm.avail_out = (uInt)out.size();
				if (inflateInit2(&strm, -MAX_WBITS) != Z_OK) return false;
				int r = inflate(&strm, Z_FINISH);
				inflateEnd(&strm);
				if (r != Z_STREAM_END) return false;
				out.resize(strm.total_out);
				return true;
			}
			return false;
		}
		return false;
	}

	unsigned int uploadPcm(const void* data, int frames, int channels, int rate) {
		if (silent || frames <= 0 || channels <= 0) return 0;
		ALenum format = (channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
		ALuint buffer = 0;
		alGenBuffers(1, &buffer);
		alBufferData(buffer, format, data, frames * channels * (int)sizeof(short), rate);
		buffers[buffer] = true;
		return buffer;
	}

	unsigned int decodeToBuffer(const unsigned char* data, int size) {
		if (!data || size < 12) return 0;
		if (data[0] == 'R' && data[1] == 'I' && data[2] == 'F' && data[3] == 'F') {
			int channels = 0, rate = 0, bits = 0;
			const unsigned char* pcm = 0;
			int pcmBytes = 0;
			size_t off = 12;
			while (off + 8 <= (size_t)size) {
				uint32_t chunk = ru32(data + off);
				uint32_t len = ru32(data + off + 4);
				off += 8;
				if (off + len > (size_t)size) break;
				if (chunk == 0x20746d66u && len >= 16) { // "fmt "
					channels = ru16(data + off + 2);
					rate = (int)ru32(data + off + 4);
					bits = ru16(data + off + 14);
				} else if (chunk == 0x61746164u) { // "data"
					pcm = data + off;
					pcmBytes = (int)len;
				}
				off += len + (len & 1);
			}
			if (!pcm || channels <= 0 || rate <= 0) return 0;
			if (bits == 16) {
				return uploadPcm(pcm, pcmBytes / (channels * 2), channels, rate);
			}
			if (bits == 8) {
				std::vector<short> wide(pcmBytes);
				for (int i = 0; i < pcmBytes; ++i) wide[i] = (short)(((int)pcm[i] - 128) << 8);
				return uploadPcm(&wide[0], pcmBytes / channels, channels, rate);
			}
			return 0;
		}
		int channels = 0, rate = 0;
		short* pcm = 0;
		int frames = stb_vorbis_decode_memory(data, size, &channels, &rate, &pcm);
		if (frames <= 0 || !pcm) return 0;
		unsigned int buffer = uploadPcm(pcm, frames, channels, rate);
		free(pcm);
		return buffer;
	}

	Voice* voiceOf(unsigned int sound) {
		std::map<unsigned int, Voice>::iterator it = voices.find(sound);
		if (it == voices.end() || !it->second.alive) return 0;
		return &it->second;
	}

} // namespace

	bool fslInit(int system) {
		std::lock_guard<std::recursive_mutex> lock(mutex);
		if (ready) return true;
		if (system == FSL_SS_DIRECTSOUND3D || system == FSL_SS_DIRECTSOUND || system == FSL_SS_MMSYSTEM)
			return false;
		if (system == FSL_SS_NOSYSTEM) {
			silent = true;
			ready = true;
			return true;
		}
		device = alcOpenDevice(0);
		if (!device) return false;
		context = alcCreateContext(device, 0);
		if (!context) {
			alcCloseDevice(device);
			device = 0;
			return false;
		}
		alcMakeContextCurrent(context);
		alDistanceModel(AL_INVERSE_DISTANCE);
		alListenerf(AL_GAIN, masterVolume);
		silent = false;
		ready = true;
		return true;
	}

	void fslShutDown() {
		std::lock_guard<std::recursive_mutex> lock(mutex);
		if (!silent) {
			for (std::map<unsigned int, Voice>::iterator it = voices.begin(); it != voices.end(); ++it) {
				if (it->second.source) {
					alSourceStop(it->second.source);
					alDeleteSources(1, &it->second.source);
				}
			}
			for (std::map<unsigned int, bool>::iterator it = buffers.begin(); it != buffers.end(); ++it) {
				ALuint buffer = it->first;
				alDeleteBuffers(1, &buffer);
			}
			if (context) {
				alcMakeContextCurrent(0);
				alcDestroyContext(context);
				context = 0;
			}
			if (device) {
				alcCloseDevice(device);
				device = 0;
			}
		}
		voices.clear();
		buffers.clear();
		ready = false;
		silent = false;
	}

	void fslSetAutoUpdate(bool) {
		// OpenAL Soft mixes on its own thread.
	}

	void fslEnterCriticalSection() { mutex.lock(); }
	void fslLeaveCriticalSection() { mutex.unlock(); }

	unsigned int fslCreateEmptySound() {
		std::lock_guard<std::recursive_mutex> lock(mutex);
		Voice voice;
		voice.source = 0;
		voice.streaming = false;
		voice.alive = true;
		voice.baseGain = 1.f;
		voice.gainMul = 1.f;
		voice.basePitch = 1.f;
		voice.pitchMul = 1.f;
		unsigned int id = 0;
		if (!silent && ready) {
			alGenSources(1, &voice.source);
			id = voice.source;
			applyGain(voice);
			applyPitch(voice);
		} else {
			id = nextFake++;
		}
		voices[id] = voice;
		return id;
	}

	unsigned int fslLoadBuffer(const char* path) {
		std::vector<unsigned char> file;
		if (!readFile(path, file)) return 0;
		std::lock_guard<std::recursive_mutex> lock(mutex);
		return decodeToBuffer(&file[0], (int)file.size());
	}

	unsigned int fslLoadBufferFromZip(const char* zipPath, const char* entryName) {
		std::vector<unsigned char> zip, raw;
		if (!readFile(zipPath, zip)) return 0;
		if (!unzipEntry(zip, entryName, raw) || raw.empty()) return 0;
		std::lock_guard<std::recursive_mutex> lock(mutex);
		return decodeToBuffer(&raw[0], (int)raw.size());
	}

	unsigned int fslStreamSound(const char* path) {
		unsigned int buffer = fslLoadBuffer(path);
		unsigned int sound = fslCreateEmptySound();
		if (buffer) fslSetSoundBuffer(sound, buffer);
		std::lock_guard<std::recursive_mutex> lock(mutex);
		Voice* voice = voiceOf(sound);
		if (voice) voice->streaming = true;
		return sound;
	}

	void fslSetSoundBuffer(FSLsound sound, unsigned int buffer) {
		std::lock_guard<std::recursive_mutex> lock(mutex);
		Voice* voice = voiceOf(sound);
		if (!voice || silent || !voice->source) return;
		alSourcei(voice->source, AL_BUFFER, (ALint)buffer);
	}

	unsigned int fslGetBufferFromSound(FSLsound sound) {
		std::lock_guard<std::recursive_mutex> lock(mutex);
		Voice* voice = voiceOf(sound);
		if (!voice || silent || !voice->source) return 0;
		ALint buffer = 0;
		alGetSourcei(voice->source, AL_BUFFER, &buffer);
		return (unsigned int)buffer;
	}

	void fslFreeSound(FSLsound sound, bool removeBuffer) {
		std::lock_guard<std::recursive_mutex> lock(mutex);
		Voice* voice = voiceOf(sound);
		if (!voice) return;
		unsigned int buffer = 0;
		if (!silent && voice->source) {
			ALint attached = 0;
			alGetSourcei(voice->source, AL_BUFFER, &attached);
			buffer = (unsigned int)attached;
			alSourceStop(voice->source);
			alSourcei(voice->source, AL_BUFFER, 0);
			alDeleteSources(1, &voice->source);
		}
		voice->alive = false;
		voices.erase(sound);
		if (removeBuffer && buffer) {
			ALuint id = buffer;
			if (!silent) alDeleteBuffers(1, &id);
			buffers.erase(buffer);
		}
	}

	void fslFreeAllBuffers() {
		std::lock_guard<std::recursive_mutex> lock(mutex);
		if (!silent) {
			for (std::map<unsigned int, Voice>::iterator it = voices.begin(); it != voices.end(); ++it) {
				if (it->second.source) alSourcei(it->second.source, AL_BUFFER, 0);
			}
			for (std::map<unsigned int, bool>::iterator it = buffers.begin(); it != buffers.end(); ++it) {
				ALuint buffer = it->first;
				alDeleteBuffers(1, &buffer);
			}
		}
		buffers.clear();
	}

	void fslSoundStop(FSLsound sound) {
		std::lock_guard<std::recursive_mutex> lock(mutex);
		Voice* voice = voiceOf(sound);
		if (voice && !silent && voice->source) alSourceStop(voice->source);
	}

	void fslSoundStopAllSounds(bool stopStreams) {
		std::lock_guard<std::recursive_mutex> lock(mutex);
		for (std::map<unsigned int, Voice>::iterator it = voices.begin(); it != voices.end(); ++it) {
			if (!stopStreams && it->second.streaming) continue;
			if (!silent && it->second.source) alSourceStop(it->second.source);
		}
	}

	void fslSoundPlaySynch(unsigned int count, FSLsound* sounds) {
		std::lock_guard<std::recursive_mutex> lock(mutex);
		if (silent || !sounds) return;
		std::vector<ALuint> sources;
		sources.reserve(count);
		for (unsigned int i = 0; i < count; ++i) {
			Voice* voice = voiceOf(sounds[i]);
			if (voice && voice->source) sources.push_back(voice->source);
		}
		if (!sources.empty()) alSourcePlayv((ALsizei)sources.size(), &sources[0]);
	}

	bool fslSoundIsInactive(FSLsound sound) {
		std::lock_guard<std::recursive_mutex> lock(mutex);
		Voice* voice = voiceOf(sound);
		if (!voice) return true;
		if (silent || !voice->source) return true;
		ALint state = AL_STOPPED;
		alGetSourcei(voice->source, AL_SOURCE_STATE, &state);
		return state != AL_PLAYING;
	}

	bool fslSoundIsStreaming(FSLsound sound) {
		std::lock_guard<std::recursive_mutex> lock(mutex);
		Voice* voice = voiceOf(sound);
		return voice && voice->streaming;
	}

	void fslSoundSetLooping(FSLsound sound, bool loop) {
		std::lock_guard<std::recursive_mutex> lock(mutex);
		Voice* voice = voiceOf(sound);
		if (voice && !silent && voice->source) alSourcei(voice->source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
	}

	void fslSoundSetGain(FSLsound sound, float gain, float multiplier) {
		std::lock_guard<std::recursive_mutex> lock(mutex);
		Voice* voice = voiceOf(sound);
		if (!voice) return;
		voice->baseGain = gain;
		voice->gainMul = multiplier;
		applyGain(*voice);
	}

	float fslSoundGetGain(FSLsound sound) {
		std::lock_guard<std::recursive_mutex> lock(mutex);
		Voice* voice = voiceOf(sound);
		return voice ? voice->baseGain : 0.f;
	}

	void fslSoundSetPitch(FSLsound sound, float pitch, float multiplier) {
		std::lock_guard<std::recursive_mutex> lock(mutex);
		Voice* voice = voiceOf(sound);
		if (!voice) return;
		voice->basePitch = pitch;
		voice->pitchMul = multiplier;
		applyPitch(*voice);
	}

	float fslSoundGetPitch(FSLsound sound) {
		std::lock_guard<std::recursive_mutex> lock(mutex);
		Voice* voice = voiceOf(sound);
		return voice ? voice->basePitch : 1.f;
	}

	void fslSoundSetPosition(FSLsound sound, float x, float y, float z) {
		std::lock_guard<std::recursive_mutex> lock(mutex);
		Voice* voice = voiceOf(sound);
		if (voice && !silent && voice->source) alSource3f(voice->source, AL_POSITION, x, y, z);
	}

	void fslSoundSetReferenceDistance(FSLsound sound, float distance) {
		std::lock_guard<std::recursive_mutex> lock(mutex);
		Voice* voice = voiceOf(sound);
		if (voice && !silent && voice->source) alSourcef(voice->source, AL_REFERENCE_DISTANCE, distance);
	}

	void fslSoundSetRolloff(FSLsound sound, float rolloff) {
		std::lock_guard<std::recursive_mutex> lock(mutex);
		Voice* voice = voiceOf(sound);
		if (voice && !silent && voice->source) alSourcef(voice->source, AL_ROLLOFF_FACTOR, rolloff);
	}

	void fslSoundSetMaxDistance(FSLsound sound, float distance) {
		std::lock_guard<std::recursive_mutex> lock(mutex);
		Voice* voice = voiceOf(sound);
		if (voice && !silent && voice->source) alSourcef(voice->source, AL_MAX_DISTANCE, distance);
	}

	void fslSetListenerDistanceModel(FSLenum model) {
		std::lock_guard<std::recursive_mutex> lock(mutex);
		if (silent) return;
		ALenum alModel = AL_INVERSE_DISTANCE;
		switch (model) {
			case FSL_DISTANCE_INVERSE_CLAMPED: alModel = AL_INVERSE_DISTANCE_CLAMPED; break;
			case FSL_DISTANCE_LINEAR: alModel = AL_LINEAR_DISTANCE; break;
			case FSL_DISTANCE_LINEAR_CLAMPED: alModel = AL_LINEAR_DISTANCE_CLAMPED; break;
			case FSL_DISTANCE_EXPONENT: alModel = AL_EXPONENT_DISTANCE; break;
			case FSL_DISTANCE_EXPONENT_CLAMPED: alModel = AL_EXPONENT_DISTANCE_CLAMPED; break;
			case FSL_DISTANCE_INVERSE:
			default: alModel = AL_INVERSE_DISTANCE; break;
		}
		alDistanceModel(alModel);
	}

	void fslSetListenerPosition(float x, float y, float z) {
		std::lock_guard<std::recursive_mutex> lock(mutex);
		if (!silent) alListener3f(AL_POSITION, x, y, z);
	}

	void fslSetListenerOrientation(float atx, float aty, float atz, float upx, float upy, float upz) {
		std::lock_guard<std::recursive_mutex> lock(mutex);
		if (silent) return;
		float ori[6] = { atx, aty, atz, upx, upy, upz };
		alListenerfv(AL_ORIENTATION, ori);
	}

	void fslSetListenerVelocity(float x, float y, float z) {
		std::lock_guard<std::recursive_mutex> lock(mutex);
		if (!silent) alListener3f(AL_VELOCITY, x, y, z);
	}

	void fslSetVolume(float gain) {
		std::lock_guard<std::recursive_mutex> lock(mutex);
		masterVolume = gain;
		if (!silent) alListenerf(AL_GAIN, masterVolume);
	}

	const float& fslGetVolume() {
		return masterVolume;
	}

} // namespace FreeSL
