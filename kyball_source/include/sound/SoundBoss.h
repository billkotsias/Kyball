/////////////
// SoundBoss
/////////////

#pragma once
#ifndef SoundBoss_H
#define SoundBoss_H

#include "FreeSL.h"
#include "tween\TPlayed.h"

#include <OgreVector3.h>

#include <map>
#include <vector>
#include <deque>
#include <string>

namespace Ogre {
	class Camera;
}

namespace P3D {

	class SoundBoss : public TPlayed {

	private:

		SoundBoss(std::string _path, std::string _ext, unsigned int, unsigned int, unsigned int);
		static SoundBoss* instance;
		~SoundBoss();

	protected:

		/// tweener playlists
		float masterVolumePreFade; /// save to restore after fade-all
		float musicVolumePreFade; /// save to restore after fade-music

		std::vector<FreeSL::FSLsound> music;
		std::vector<FreeSL::FSLsound> ambience;

		std::deque<FreeSL::FSLsound> inactiveSounds;	/// inactive voices
		std::deque<FreeSL::FSLsound> lowSounds;			/// } playing voices, categorized by priority (co-operative)
		std::deque<FreeSL::FSLsound> highSounds;		/// }

		std::map<FreeSL::FSLsound, bool> validHandles;	/// keep record of valid handles...

		unsigned int createEmptySound() {
			unsigned int voice = FreeSL::fslCreateEmptySound();
			validHandles[voice] = true;
			return voice;
		};

		template<class Class>
		void registerSoundGroup(Class& group, unsigned int voices) {
			group.resize(voices);
			for (unsigned int i = 0; i < voices; ++i) {
				group.at(i) = createEmptySound();
			}
		};

		// volumes
		float musicVolume;
		float ambienceVolume;
		float soundsVolume;

		template<class Class>
		void setGroupVolume(const Class& group, const float& vol) {
			for (int i = group.size() - 1; i >= 0; --i) {
				FreeSL::FSLsound sound = group.at(i);
				FreeSL::fslSoundSetGain(sound, FreeSL::fslSoundGetGain(sound), vol);
			}
		};

		// pitch
		float musicPitch;
		float ambiencePitch;
		float soundsPitch;

		template<class Class>
		void setGroupPitch(const Class& group, const float& pit) {
			for (int i = group.size() - 1; i >= 0; --i) {
				FreeSL::FSLsound sound = group.at(i);
				FreeSL::fslSoundSetPitch(sound, FreeSL::fslSoundGetPitch(sound), pit);
			}
		};

		// sound loading + playing
		std::string path;
		std::string ext;
		std::map<std::string, unsigned int> loadedBuffers;
		Ogre::Camera* camera;

		/// load sound
		/// <= pointer
		unsigned int loadSound(const char* strFile1, const char* strFile2 = 0); /// provide second file if in a .zip file

		/// helpers
		template<class Class>
		std::vector<FreeSL::FSLsound*> getInactiveVoices(Class& group, int num) {
			std::vector<FreeSL::FSLsound*> voices;
			if (num == 0) return voices;

			for (unsigned int i = 0; i < group.size(); ++i) {
				FreeSL::FSLsound& voice = group.at(i);
				if (FreeSL::fslSoundIsInactive(voice)) {
					voices.push_back(&voice);
					if (voices.size() == num) break;
				}
			}

			return voices;
		};

		template <class Class>
		std::vector<FreeSL::FSLsound> acquireVoices(Class& group, bool highPriority);

		void _setSoundBuffers( std::vector<FreeSL::FSLsound>& voices, const std::vector<unsigned int>& buffers );
		void _setSoundBuffers( std::vector<FreeSL::FSLsound*>& voices, const std::vector<unsigned int>& buffers );
		void _setSoundStreams( std::vector<FreeSL::FSLsound*>& voices, const std::vector<std::string>& streams );

		template<class Class>
		std::vector<Class> _dereference( const std::vector<Class*>& pointers ) {
			std::vector<Class> values;
			for (unsigned int i = 0; i < pointers.size(); ++i) {
				values.push_back( *(pointers.at(i)) );
			}
			return values;
		};

		template<class Class>
		std::vector<Class*> _reference( std::vector<Class>& values ) {
			std::vector<Class*> pointers;
			for (unsigned int i = 0; i < values.size(); ++i) {
				pointers.push_back( &(values.at(i)) );
			}
			return pointers;
		};

		void _playVoices(
			const std::vector<FreeSL::FSLsound>& voices, const float& vol, const float& pit, const float& mVol, const float& mPit,
			const Ogre::Vector3& pos, const bool& loop, const float& refDist, const float& rolloff, const float& maxDist
			);

		// music queueing
		std::vector<std::string> musicQueue; /// waiting for current music to fade-out
		float musicQueueVol;
		float musicQueuePitch;

		// update helpers
		void discoverInactiveVoices(std::deque<FreeSL::FSLsound>& group);

		// the title says it all
		bool isHandleValid(unsigned int);

		/// helper
		template<class Class>
		bool isHandleIn(unsigned int handle, Class& group) {
			for (unsigned int i = 0; i < group.size(); ++i) {
				if (handle == group.at(i)) return true;
			}
			return false;
		};

		void _setMasterVolume(const float&);		/// } hack Bill's inconsistency

		void musicFadeFinished();

	public:

		enum {
			FADE_ALL = 1, /// musn't start from 0 <!>
			FADE_MUSIC,
		};

		static const std::string MUSIC_EXT;

		float worldScale;			/// scales distances
		double musicFadeoutTime;	/// time to fade-out previously playing music
		double musicFadeinTime;		/// time to fade-in new music

		// constructor
		// => define number of voices for each sound group
		static SoundBoss* init(std::string _path, std::string _ext, unsigned int mus = 1, unsigned int ambi = 3, unsigned int max = 16);
		static SoundBoss* getSingletonPtr();
		static void destroy();

		// provide sound pointer (buffer); load if not already in memory
		// <= pointer
		unsigned int* getBuffer(const char* strFile1, const char* strFile2 = 0); /// provide second file if in a .zip file

		// set "listener"
		void setCamera(Ogre::Camera* cam, FreeSL::FSLenum enu = FSL_INVERSE_DISTANCE);

		// play sounds synchronized
		// => pointer(s)
		//	  volume + pitch multipliers
		// <= pointer(s) to voice(s) activated
		std::vector<unsigned int> playMusic(const std::vector<unsigned int>&, const float& vol = 1.f, const float& pit = 1.f);
		std::vector<unsigned int> streamMusic(const std::vector<std::string>&, const float& vol = 1.f, const float& pit = 1.f);

		unsigned int playAmbience(const unsigned int& buffer, const Ogre::Vector3& pos, const float& refDist, const float& rolloff,
		const float& maxDist, const float& vol, const float& pit);
		std::vector<unsigned int> playAmbience(const std::vector<unsigned int>& buffers, const Ogre::Vector3& pos, const float& refDist, const float& rolloff,
		const float& maxDist, const float& vol, const float& pit);

		unsigned int streamAmbience(const std::string& stream, const Ogre::Vector3& pos, const float& refDist, const float& rolloff,
		const float& maxDist, const float& vol, const float& pit);
		std::vector<unsigned int> streamAmbience(const std::vector<std::string>& streams, const Ogre::Vector3& pos, const float& refDist, const float& rolloff,
		const float& maxDist, const float& vol, const float& pit);

		std::vector<unsigned int> playSounds(const std::vector<unsigned int>&, bool highPriority,
			const Ogre::Vector3& pos, const float& refDist, const float& rolloff, const float& maxDist,
			 const float& vol = 1.f, const float& pit = 1.f
			 );

		unsigned int playSound(const unsigned int&, bool highPriority,
			const Ogre::Vector3& pos, const float& refDist, const float& rolloff, const float& maxDist,
			const float& vol = 1.f, const float& pit = 1.f
			);

		std::vector<unsigned int> streamSounds(const std::vector<std::string>&, bool highPriority,
			const Ogre::Vector3& pos, const float& refDist, const float& rolloff, const float& maxDist,
			 const float& vol = 1.f, const float& pit = 1.f
			 );

		unsigned int streamSound(const std::string&, bool highPriority,
			const Ogre::Vector3& pos, const float& refDist, const float& rolloff, const float& maxDist,
			const float& vol = 1.f, const float& pit = 1.f
			);

		// replace music with another; previous is faded-out in 'musicFadeoutTime' secs
		std::vector<unsigned int> newStreamMusic(const std::vector<std::string>&, const float& vol = 1.f, const float& pit = 1.f);
		unsigned int newStreamMusic(const std::string&, const float& vol = 1.f, const float& pit = 1.f);

		// is sound playing?
		// => pointer
		bool isSoundPlaying(unsigned int);

		// stop a sound
		// => pointer
		void stopSound(unsigned int);

		// free a sound
		// => free buffer too? WORKS ONLY IF NO OTHER VOICE IS PLAYING THIS BUFFER!!!
		void freeSound(unsigned int, bool buffer = false);

		// stop all playing sounds and free all buffers
		// => stop streaming ones, too?
		void freeAllSounds(bool stopStreams);

		// must be called every frame (or faster! for slow fps)
		void update();

		// volumes
		void fadeAll(double time = 1.);
		void setMasterVolume(const float&);		/// } hack FreeSL's inconsistency
		const float& getMasterVolume() const;	/// }

		void setMusicVolume(const float&);
		void setAmbienceVolume(const float&);
		void setSoundsVolume(const float&);

		const float& getMusicVolume() const { return musicVolume; };
		const float& getAmbienceVolume() const { return ambienceVolume; };
		const float& getSoundsVolume() const { return soundsVolume; };

		// pitch
		void setMusicPitch(const float&);
		void setAmbiencePitch(const float&);
		void setSoundsPitch(const float&);

		// overrides
		virtual void playlistFinished(TPlaylist*);
	};
}

#endif