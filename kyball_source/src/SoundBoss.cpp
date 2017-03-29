#include "StdAfx.h"
#include "sound\SoundBoss.h"

#include "tween\TLinear.h"

#include <math\Math.h>

namespace P3D {

	const std::string SoundBoss::MUSIC_EXT = ".ogg";

	// Singleton logic
	SoundBoss* SoundBoss::instance = 0;

	SoundBoss* SoundBoss::init(std::string _path, std::string _ext, unsigned int mus, unsigned int ambi, unsigned int max) {
		if (instance) throw "SoundBoss is already initialized!\n";
		instance = new SoundBoss(_path, _ext, mus, ambi, max);
		return instance;
	}

	SoundBoss* SoundBoss::getSingletonPtr() {
		return instance;
	}

	SoundBoss::SoundBoss(std::string _path, std::string _ext, unsigned int musicVoices, unsigned int ambienceVoices, unsigned int maxVoices)
		: path(_path), ext(_ext), masterVolumePreFade(-1.), musicVolumePreFade(-1.), musicFadeoutTime(1.), musicFadeinTime(2.), camera(0), worldScale(1.f) {

		/// initialize FreeSL with a system preference order
		if (!FreeSL::fslInit(FreeSL::FSL_SS_DIRECTSOUND3D))
			if (!FreeSL::fslInit(FreeSL::FSL_SS_DIRECTSOUND))
				if (!FreeSL::fslInit(FreeSL::FSL_SS_MMSYSTEM))
					if (!FreeSL::fslInit(FreeSL::FSL_SS_ALUT))
						FreeSL::fslInit(FreeSL::FSL_SS_NOSYSTEM);

		/// set separate thread for auto-update
		FreeSL::fslSetAutoUpdate(true);
		setCamera(0);

		int soundVoices = maxVoices - musicVoices - ambienceVoices;

		/// register sound groups
		registerSoundGroup(music, musicVoices);
		registerSoundGroup(ambience, ambienceVoices);
		registerSoundGroup(inactiveSounds, soundVoices);

		musicVolume = 1.f;
		ambienceVolume = 1.f;
		soundsVolume = 1.f;
		musicPitch = 1.f;
		ambiencePitch = 1.f;
		soundsPitch = 1.f;
	}

	SoundBoss::~SoundBoss() {
		instance = 0;

		/// TPlayed
		deleteTweens();

		/// FreeSL
		FreeSL::fslShutDown();
	}

	void SoundBoss::destroy() {
		delete instance;
		instance = 0;
	}
	// Singleton logic

	/// music
	std::vector<unsigned int> SoundBoss::playMusic(const std::vector<unsigned int>& buffers, const float& vol, const float& pit) {
		std::vector<FreeSL::FSLsound*> pVoices = getInactiveVoices(music, buffers.size());
		_setSoundBuffers(pVoices, buffers);

		std::vector<FreeSL::FSLsound> voices = _dereference(pVoices);
		_playVoices(voices, vol, pit, musicVolume, musicPitch, Ogre::Vector3::ZERO, true, Infinitef, 1.f, Infinitef);
		return voices;
	}

	std::vector<unsigned int> SoundBoss::streamMusic(const std::vector<std::string>& streams, const float& vol, const float& pit) {
		std::vector<FreeSL::FSLsound*> pVoices = getInactiveVoices(music, streams.size());
		_setSoundStreams(pVoices, streams);

		std::vector<FreeSL::FSLsound> voices = _dereference(pVoices);
		_playVoices(voices, vol, pit, musicVolume, musicPitch, Ogre::Vector3::ZERO, true, Infinitef, 1.f, Infinitef);
		return voices;
	}

	unsigned int SoundBoss::newStreamMusic(const std::string& stream, const float& vol, const float& pit) {
		std::vector<std::string> streams;
		streams.push_back(stream);

		std::vector<unsigned int> voices = newStreamMusic(streams, vol, pit);
		if (voices.empty()) return 0;
		return voices.at(0);
	}

	std::vector<unsigned int> SoundBoss::newStreamMusic(const std::vector<std::string>& streams, const float& vol, const float& pit)
	{
		bool playingMusic = false;
		for (unsigned int i = 0; i < music.size(); ++i) {
			if (!FreeSL::fslSoundIsInactive(music.at(i))) {
				playingMusic = true;
				break;
			}
		}
		//std::cout << "New Streams=" << streams.at(0) << ":" << playingMusic << "\n";
		if (!playingMusic) {
			/// Finally, nothing is played! Now fade-in new music!
			double musicVolumePostFade = musicVolume;
			setMusicVolume(0);
			TPlaylist* fadeMusicList = registerTween(new TLinear<P3D::SoundBoss, float>(
				this, musicVolumePostFade, musicFadeinTime, &P3D::SoundBoss::getMusicVolume, &P3D::SoundBoss::setMusicVolume)
				);
			return streamMusic(streams, vol, pit); /// BYE!
		}

		/// Hmmm, must fade-out current music 1st!
		if (musicQueue == streams) {
			return std::vector<unsigned int>(); /// not so fast! Wait!
		}
		musicQueue = streams;
		musicQueueVol = vol;
		musicQueuePitch = pit;

		if (musicVolumePreFade < 0) deleteTweens(); /// >= 0 indicates fading-all is already running!
		if ( deleteID(FADE_MUSIC) ) {
			musicFadeFinished(); /// force finish of already-hapenning fade-out
		}

		musicVolumePreFade = musicVolume;
		TPlaylist* fadeMusicList = registerTween(new TLinear<P3D::SoundBoss, float>(
			this, 0., musicFadeoutTime, &P3D::SoundBoss::getMusicVolume, &P3D::SoundBoss::setMusicVolume)
			);
		fadeMusicList->id = FADE_MUSIC;

		return std::vector<unsigned int>(); /// is empty, as it hasn't started playing yet!
	}

	/// looped sounds (ambience)
	unsigned int SoundBoss::playAmbience(const unsigned int& buffer, const Ogre::Vector3& pos, const float& refDist, const float& rolloff,
		const float& maxDist, const float& vol, const float& pit)
	{
		std::vector<unsigned int> buffers;
		buffers.push_back(buffer);

		std::vector<unsigned int> voices = playAmbience(buffers, pos, refDist, rolloff, maxDist, vol, pit);
		if (voices.empty()) return 0;
		return voices.at(0);
	}
	std::vector<unsigned int> SoundBoss::playAmbience(const std::vector<unsigned int>& buffers, const Ogre::Vector3& pos, const float& refDist, const float& rolloff,
		const float& maxDist, const float& vol, const float& pit)
	{
		std::vector<FreeSL::FSLsound*> pVoices = getInactiveVoices(ambience, buffers.size());
		_setSoundBuffers(pVoices, buffers);

		std::vector<FreeSL::FSLsound> voices = _dereference(pVoices);
		_playVoices(voices, vol, pit, ambienceVolume, ambiencePitch, pos, true, refDist, rolloff, maxDist);
		return voices;
	}

	unsigned int SoundBoss::streamAmbience(const std::string& stream, const Ogre::Vector3& pos, const float& refDist, const float& rolloff,
		const float& maxDist, const float& vol, const float& pit)
	{
		std::vector<std::string> streams;
		streams.push_back(stream);

		std::vector<unsigned int> voices = streamAmbience(streams, pos, refDist, rolloff, maxDist, vol, pit);
		if (voices.empty()) return 0;
		return voices.at(0);
	}
	std::vector<unsigned int> SoundBoss::streamAmbience(const std::vector<std::string>& streams, const Ogre::Vector3& pos, const float& refDist, const float& rolloff,
		const float& maxDist, const float& vol, const float& pit)
	{
		std::vector<FreeSL::FSLsound*> pVoices = getInactiveVoices(ambience, streams.size());
		_setSoundStreams(pVoices, streams);

		std::vector<FreeSL::FSLsound> voices = _dereference(pVoices);
		_playVoices(voices, vol, pit, ambienceVolume, ambiencePitch, pos, true, refDist, rolloff, maxDist);
		return voices;
	}

	template <class Class>
	std::vector<FreeSL::FSLsound> SoundBoss::acquireVoices(Class& group, bool highPriority) {

		std::vector<FreeSL::FSLsound> voices;

		/// grab as many inactive sounds as possible
		while (voices.size() < group.size() && !inactiveSounds.empty()) {
			voices.push_back( inactiveSounds.front() );
			inactiveSounds.pop_front();
			//voices.push_back( inactiveSounds.back() ); /// don't know why this didn't work; maybe bug in FreeSL or OpenAL
			//inactiveSounds.pop_back();
		}

		/// not enough voices aquired ?!
		/// - <1st> solution : grab <oldest> low-priority sounds / <HACK> : DON'T TOUCH STREAMING VOICES IF STILL PLAYING!
		{
			unsigned int i = 0;
			while (voices.size() < group.size() && i < lowSounds.size()) {
				FreeSL::FSLsound voice = lowSounds.at(i);
				if (FreeSL::fslSoundIsStreaming(voice)) {
					++i;
				} else {
					//std::cout << "SB:acquireVoices\n";
					stopSound(voice); /// <!>
					voices.push_back(voice);
					lowSounds.erase(lowSounds.begin() + i);
				}
			}
		}

		/// still not enough voices aquired ?!
		/// - <2nd> solution : if new sounds are high-priority, grab <oldest> high-priority sounds - <VERY UNCOMMON CASE> to have so many high-priority sounds!
		{
			unsigned int i = 0;
			if (highPriority) {
				while (voices.size() < group.size() && i < highSounds.size()) {
					FreeSL::FSLsound voice = highSounds.at(i);
					if (FreeSL::fslSoundIsStreaming(voice)) {
						++i;
					} else {
						//std::cout << "SB:acquireVoices2\n";
						stopSound(voice); /// <!>
						voices.push_back(voice);
						highSounds.erase(highSounds.begin() + i);
					}
				}
			}
		}

		return voices;
	}

	std::vector<unsigned int> SoundBoss::playSounds(const std::vector<unsigned int>& buffers, bool highPriority,
		const Ogre::Vector3& pos, const float& refDist, const float& rolloff, const float& maxDist, const float& vol, const float& pit) {

		std::vector<FreeSL::FSLsound> voices = acquireVoices(buffers, highPriority);
		_setSoundBuffers(voices, buffers); /// --- here, 'voices' are <altered> !!!
		//_setSoundBuffers(_reference(voices), buffers); /// --- here, 'voices' are <altered> !!!

		/// register voices to respective priority group
		if (highPriority) {
			highSounds.insert( highSounds.end(), voices.begin(), voices.end() );
		} else {
			lowSounds.insert( lowSounds.end(), voices.begin(), voices.end() );
		}

		_playVoices(voices, vol, pit, soundsVolume, soundsPitch, pos, false, refDist, rolloff, maxDist);

		return voices;
	}

	unsigned int SoundBoss::playSound(const unsigned int& buffer, bool highPriority,
		const Ogre::Vector3& pos, const float& refDist, const float& rolloff, const float& maxDist, const float& vol, const float& pit) {

		std::vector<unsigned int> buffers;
		buffers.push_back(buffer);

		std::vector<unsigned int> voices = playSounds(buffers, highPriority, pos, refDist, rolloff, maxDist, vol, pit);
		if (voices.empty()) return 0;
		return voices.at(0);
	}

	std::vector<unsigned int> SoundBoss::streamSounds(const std::vector<std::string>& streams, bool highPriority,
		const Ogre::Vector3& pos, const float& refDist, const float& rolloff, const float& maxDist, const float& vol, const float& pit) {

		std::vector<FreeSL::FSLsound> voices = acquireVoices(streams, highPriority);
		_setSoundStreams(_reference(voices), streams); /// --- here, 'voices' are <altered> !!!

		/// register voices to respective priority group
		if (highPriority) {
			highSounds.insert( highSounds.end(), voices.begin(), voices.end() );
		} else {
			lowSounds.insert( lowSounds.end(), voices.begin(), voices.end() );
		}

		_playVoices(voices, vol, pit, soundsVolume, soundsPitch, pos, false, refDist, rolloff, maxDist);

		return voices;
	}

	unsigned int SoundBoss::streamSound(const std::string& stream, bool highPriority,
		const Ogre::Vector3& pos, const float& refDist, const float& rolloff, const float& maxDist, const float& vol, const float& pit) {

		std::vector<std::string> streams;
		streams.push_back(stream);

		std::vector<unsigned int> voices = streamSounds(streams, highPriority, pos, refDist, rolloff, maxDist, vol, pit);
		if (voices.empty()) return 0;
		return voices.at(0);
	}

	void SoundBoss::stopSound(unsigned int voice) {
		//std::cout << "SB:stopSound" << voice << "\n";
		if (!isHandleValid(voice)) return;

		FreeSL::fslEnterCriticalSection();
		FreeSL::fslSoundStop(voice);
		FreeSL::fslLeaveCriticalSection();
	}

	void SoundBoss::freeSound(unsigned int voice, bool removeBuf) {
		//std::cout << "SB:freeSound" << voice << removeBuf << "\n";
		if (!isHandleValid(voice)) return;

		FreeSL::fslEnterCriticalSection();
		FreeSL::fslSoundStop(voice);
		if (removeBuf) {
			/// very slow way to remove a buffer, also make sure that <NO OTHER VOICE IS PLAYING THE BUFFER OR IT WON'T BE REMOVED!!!>
			unsigned int buffer = FreeSL::fslGetBufferFromSound(voice);
			for (std::map<std::string, unsigned int>::iterator it = loadedBuffers.begin(); it != loadedBuffers.end(); ++it) {
				if (it->second == buffer) {
					it->second = 0;
					break;
				}
			}
		}
		FreeSL::fslFreeSound(voice, removeBuf);
		validHandles.erase(voice);
		FreeSL::fslLeaveCriticalSection();
	}

	void SoundBoss::_setSoundBuffers( std::vector<FreeSL::FSLsound>& voices, const std::vector<unsigned int>& buffers ) {
		for (unsigned int i = 0; i < voices.size(); ++i) {
			FreeSL::FSLsound voice = voices.at(i);
			if ( FreeSL::fslSoundIsStreaming(voice) ) {
				freeSound(voice, true);
				voices.at(i) = createEmptySound(); /// previous sound was streaming, replace with buffered sound
				voice = voices.at(i);
			}
			FreeSL::fslSetSoundBuffer( voice, buffers.at(i) );
		}
	}

	void SoundBoss::_setSoundBuffers( std::vector<FreeSL::FSLsound*>& voices, const std::vector<unsigned int>& buffers ) {
		for (unsigned int i = 0; i < voices.size(); ++i) {
			FreeSL::FSLsound* voice = voices.at(i);
			if ( FreeSL::fslSoundIsStreaming(*voice) ) {
				freeSound(*voice, false); /// IF CRASH, RETURN THIS TO <false>
				(*voices.at(i)) = createEmptySound(); /// previous sound was streaming, replace with buffered sound
				//voice = voices.at(i); /// already set!
			}
			FreeSL::fslSetSoundBuffer( *voice, buffers.at(i) );
		}
	}

	void SoundBoss::_setSoundStreams( std::vector<FreeSL::FSLsound*>& voices, const std::vector<std::string>& streams ) {
		for (unsigned int i = 0; i < voices.size(); ++i) {
			/// create new streaming sound
			freeSound(*voices.at(i), false); /// IF CRASH, RETURN THIS TO <false>
			unsigned int voice = FreeSL::fslStreamSound( (path + streams.at(i) + MUSIC_EXT).c_str() );
			validHandles[voice] = true; /// <!!!!!!!!!!!!!!!!!!!!!!!>
			(*voices.at(i)) = voice;
		}
	}

	void SoundBoss::_playVoices(
		const std::vector<FreeSL::FSLsound>& voices, const float& vol, const float& pit, const float& mVol, const float& mPit,
		const Ogre::Vector3& pos, const bool& loop, const float& refDist, const float& rolloff, const float& maxDist) {

		int voicesNum = voices.size();
		FreeSL::FSLsound* pVoices = new FreeSL::FSLsound[voicesNum];

		/// respect number of available voices : they are always <= buffers.size
		for (int i = 0; i < voicesNum; ++i) {

			FreeSL::FSLsound voice = voices.at(i);
			pVoices[i] = voice;

			FreeSL::fslSoundSetLooping( voice, loop );
			FreeSL::fslSoundSetGain( voice, vol, mVol );
			FreeSL::fslSoundSetPitch( voice, pit, mPit );
			FreeSL::fslSoundSetPosition( voice, pos.x, pos.y, pos.z );
			FreeSL::fslSoundSetReferenceDistance( voice, refDist * worldScale );
			FreeSL::fslSoundSetRolloff( voice, rolloff );
			FreeSL::fslSoundSetMaxDistance( voice, maxDist * worldScale );
			//std::cout << "Source position = " << pos << "\n";
			//std::cout << "Camera position = " << camera->getDerivedPosition() << "\n";
			//std::cout << "Camera distance = " << camera->getDerivedPosition().distance(pos) << "\n";
			//std::cout << "max distance = " << maxDist * worldScale << " ref distance = " << refDist * worldScale << "\n";
			//std::cout << "rolloff = " << rolloff << "\n";
		}

		FreeSL::fslSoundPlaySynch(voicesNum, pVoices); /// finally, play the whole lot

		delete[] pVoices;
	}

	bool SoundBoss::isSoundPlaying(unsigned int voice) {
		if (isHandleIn(voice, lowSounds) || isHandleIn(voice, highSounds) || isHandleIn(voice, music) || isHandleIn(voice, ambience)) return true;
		return false;
	}

	bool SoundBoss::isHandleValid(unsigned int voice) {
#ifdef _DEBUG
		if (validHandles.find(voice) == validHandles.end()) {
			std::cout << "SoundBoss : Handle is not valid!!!\n";
		}
#endif
		return (validHandles.find(voice) != validHandles.end());
	}

	void SoundBoss::setCamera(Ogre::Camera* cam, FreeSL::FSLenum enu) {
		camera = cam;
		FreeSL::fslSetListenerDistanceModel(enu);
		update();
	}

	void SoundBoss::update() {

		/// - discover <inactive> sounds (low or high) and re-register as inactive
		discoverInactiveVoices(lowSounds);
		discoverInactiveVoices(highSounds);

		/// - update listener's parameters
		if (camera) {
			Ogre::Vector3 pos = camera->getDerivedPosition();
			FreeSL::fslSetListenerPosition(pos.x, pos.y, pos.z);

			Ogre::Vector3 at = camera->getDerivedDirection();
			Ogre::Vector3 up = camera->getDerivedUp();
			FreeSL::fslSetListenerOrientation(at.x, at.y, at.z, up.x, up.y, up.z);
		} else {
			FreeSL::fslSetListenerPosition(0,0,0);
			FreeSL::fslSetListenerOrientation(0,0,1,0,1,0);
		}

		FreeSL::fslSetListenerVelocity(0,0,0);
	}

	void SoundBoss::discoverInactiveVoices(std::deque<FreeSL::FSLsound>& group) {
		for (int i = group.size() - 1; i >= 0; --i) {
			FreeSL::FSLsound voice = group.at(i);
			if (FreeSL::fslSoundIsInactive(voice)) {
				group.erase( group.begin() + i);
				inactiveSounds.push_back(voice);
			}
		}
	}

	void SoundBoss::_setMasterVolume(const float& gain) {
		FreeSL::fslSetVolume(gain);
	}

	void SoundBoss::setMasterVolume(const float& gain) {
		if (masterVolumePreFade >= 0) {
			masterVolumePreFade = gain; /// we are fading right now, so set new value "post-fade"
		} else {
			_setMasterVolume(gain);
		}
	}

	const float& SoundBoss::getMasterVolume() const {
		if (masterVolumePreFade >= 0) return masterVolumePreFade; /// we are fading right now, so return "true" (pre-fade) value
		return FreeSL::fslGetVolume();
	}

	void SoundBoss::fadeAll(double time) {

		if (masterVolumePreFade >= 0) deleteTweens(); /// >= 0 indicates fading-all is already running!
		masterVolumePreFade = getMasterVolume();

		TPlaylist* fadeAllList = registerTween(new TLinear<P3D::SoundBoss, float>(
			this, 0., time, &P3D::SoundBoss::getMasterVolume, &P3D::SoundBoss::_setMasterVolume)
			);
		fadeAllList->id = FADE_ALL;
	}

	unsigned int* SoundBoss::getBuffer(const char *strFile1, const char *strFile2) {
		std::string bufferName = strFile1;
		if (strFile2) {
			bufferName += "\\";
			bufferName += strFile2;
		}
		std::map<std::string, unsigned int>::iterator it = loadedBuffers.lower_bound(bufferName);

		if( it != loadedBuffers.end() && !(loadedBuffers.key_comp()(bufferName, it->first)) )
		{
			/// key already exists
			if (!it->second) {								/// is buffer loaded?
				it->second = loadSound(strFile1, strFile2);	/// no, load buffer now (must have been freed at some point)
			}
			return &it->second;
		}
		else
		{
			/// the key does not exist in the map : create and load buffer now
			unsigned int buffer = loadSound(strFile1, strFile2);
			it = loadedBuffers.insert(it, std::map<std::string, unsigned int>::value_type(bufferName, buffer)); /// use 'it' as a hint to insert, so it can avoid another lookup
			return &it->second;
		}
			
	}

	void SoundBoss::freeAllSounds(bool stopStreams) {
		//std::cout << "SB:freeAllSounds" << stopStreams << "\n";
		FreeSL::fslEnterCriticalSection();
		FreeSL::fslSoundStopAllSounds(stopStreams);
		FreeSL::fslFreeAllBuffers();
		FreeSL::fslLeaveCriticalSection();
		for (std::map<std::string, unsigned int>::iterator it = loadedBuffers.begin(); it != loadedBuffers.end(); ++it) {
			it->second = 0; /// mark buffer as unloaded
		}
	}

	unsigned int SoundBoss::loadSound(const char *strFile1, const char *strFile2) {
		std::string filePath = path + strFile1;

		if (strFile2) {
			std::string filename = strFile2 + ext;
			return FreeSL::fslLoadBufferFromZip(filePath.c_str(), filename.c_str());
		}

		filePath += ext;
		return FreeSL::fslLoadBuffer( filePath.c_str() );
	}

	void SoundBoss::setMusicVolume(const float& vol) {
		musicVolume = vol;
		setGroupVolume(music, vol);
	}
	void SoundBoss::setAmbienceVolume(const float& vol) {
		ambienceVolume = vol;
		setGroupVolume(ambience, vol);
	}
	void SoundBoss::setSoundsVolume(const float& vol) {
		soundsVolume = vol;
		setGroupVolume(highSounds, vol);
		setGroupVolume(lowSounds, vol);
	}
	void SoundBoss::setMusicPitch(const float& pit) {
		musicPitch = pit;
		setGroupPitch(music, pit);
	}
	void SoundBoss::setAmbiencePitch(const float& pit) {
		ambiencePitch = pit;
		setGroupPitch(ambience, pit);
	}
	void SoundBoss::setSoundsPitch(const float& pit) {
		soundsPitch = pit;
		setGroupPitch(highSounds, pit);
		setGroupPitch(lowSounds, pit);
	}

	void SoundBoss::playlistFinished(TPlaylist* list) {
		//std::cout << "SB:playlistFinished:" << list->id << "\n";
		this->TPlayed::playlistFinished(list); /// <!!!>

		switch (list->id) {

			case FADE_ALL:
				freeAllSounds(true); /// <!!!>
				FreeSL::fslSetVolume(masterVolumePreFade);
				masterVolumePreFade = -1.;
				break;

			case FADE_MUSIC:
				musicFadeFinished();
				newStreamMusic(musicQueue, musicQueueVol, musicQueuePitch);
				break;
		}
	}

	void SoundBoss::musicFadeFinished() {
		for (unsigned int i = 0; i < music.size(); ++i) {
			stopSound(music.at(i));
		}
		setMusicVolume(musicVolumePreFade);
		musicVolumePreFade = -1;
	}
}