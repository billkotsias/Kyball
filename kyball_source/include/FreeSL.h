// FreeSL-compatible facade implemented on OpenAL Soft.
// SoundBoss keeps calling these entry points.

#pragma once
#ifndef KYBALL_FREESL_H
#define KYBALL_FREESL_H

namespace FreeSL {

	enum SoundSystem {
		FSL_SS_DIRECTSOUND3D = 1,
		FSL_SS_DIRECTSOUND,
		FSL_SS_MMSYSTEM,
		FSL_SS_ALUT,
		FSL_SS_NOSYSTEM
	};

	enum FSLenum {
		FSL_DISTANCE_INVERSE = 1,
		FSL_DISTANCE_INVERSE_CLAMPED,
		FSL_DISTANCE_LINEAR,
		FSL_DISTANCE_LINEAR_CLAMPED,
		FSL_DISTANCE_EXPONENT,
		FSL_DISTANCE_EXPONENT_CLAMPED
	};

	typedef unsigned int FSLsound;

	bool fslInit(int system);
	void fslShutDown();
	void fslSetAutoUpdate(bool enabled);

	void fslEnterCriticalSection();
	void fslLeaveCriticalSection();

	unsigned int fslCreateEmptySound();
	unsigned int fslLoadBuffer(const char* path);
	unsigned int fslLoadBufferFromZip(const char* zipPath, const char* entryName);
	unsigned int fslStreamSound(const char* path);

	void fslSetSoundBuffer(FSLsound sound, unsigned int buffer);
	unsigned int fslGetBufferFromSound(FSLsound sound);
	void fslFreeSound(FSLsound sound, bool removeBuffer);
	void fslFreeAllBuffers();
	void fslSoundStop(FSLsound sound);
	void fslSoundStopAllSounds(bool stopStreams);
	void fslSoundPlaySynch(unsigned int count, FSLsound* sounds);

	bool fslSoundIsInactive(FSLsound sound);
	bool fslSoundIsStreaming(FSLsound sound);

	void fslSoundSetLooping(FSLsound sound, bool loop);
	void fslSoundSetGain(FSLsound sound, float gain, float multiplier);
	float fslSoundGetGain(FSLsound sound);
	void fslSoundSetPitch(FSLsound sound, float pitch, float multiplier);
	float fslSoundGetPitch(FSLsound sound);
	void fslSoundSetPosition(FSLsound sound, float x, float y, float z);
	void fslSoundSetReferenceDistance(FSLsound sound, float distance);
	void fslSoundSetRolloff(FSLsound sound, float rolloff);
	void fslSoundSetMaxDistance(FSLsound sound, float distance);

	void fslSetListenerDistanceModel(FSLenum model);
	void fslSetListenerPosition(float x, float y, float z);
	void fslSetListenerOrientation(float atx, float aty, float atz, float upx, float upy, float upz);
	void fslSetListenerVelocity(float x, float y, float z);

	void fslSetVolume(float gain);
	const float& fslGetVolume();

}

// SoundBoss.h uses this name from the enclosing namespace.
#define FSL_INVERSE_DISTANCE ::FreeSL::FSL_DISTANCE_INVERSE

#endif
