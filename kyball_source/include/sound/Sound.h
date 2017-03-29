// -----
// Sound - High-level Sound object
// -----

#pragma once
#ifndef Sound_H
#define Sound_H

#include <OgreVector3.h>

#include <map>
#include <string>

namespace FANLib {
	class FSLClass;
}

namespace P3D {

	class Sound {

	private:

		static std::map<std::string, Sound*> allSounds;
		static Sound* defaultSound;
		static Sound* buildFromFSL(FANLib::FSLClass*);

	public:

		/// "hardcoded" sounds
		static Sound* BALL_SMALL_POP;
		static Sound* BALL_BIG_POP;
		static Sound* BALL_FIRE;
		static Sound* BALL_BOUNCE;
		static Sound* BALL_BZZZT;
		static Sound* GAME_LOSE;
		static Sound* GAME_WIN;
		static Sound* CHANGE_ENERGY;
		static Sound* GAME_WIN_GREAT;
		static Sound* CUBE_SHRINKING;
		static Sound* CUBE_SHRINK_IN1;
		static Sound* CUBE_SHRINK_IN2;

		static void readScript();						/// initialize; also sets "hardcoded" Sounds
		static void destroyAll();						/// delete all created Sounds

		/// get a sound with parameters as defined (or not) in script!
		/// =>	name = name of sound to look for
		///		base = if 'name' hasn't been scripted, then :
		///		 if { base = 0, build 'name' based on <default> script-class parameters
		///			{ base != 0, build 'name' based on <base> parameters (if 'base' doesn't exist either, itself will be based on 'default')
		static Sound* getScripted(const char* const name, const char* const base = 0);

		// instance
		std::string name;
		bool loop;
		bool priority;
		float refDist;
		float rolloff;
		float maxDist;
		float vol;
		float pitch;
		unsigned int* buffer;		/// corresponding buffer for this sound (if not streamed)
		bool streamed;				/// is this sound streamed or buffered?
		unsigned int lastPlayingVoice;	/// for convenience to the user

		// constructor
		Sound(const char* const na) : name(na), buffer(0), loop(false) {}; /// uninitialized
		Sound(const char* const na, bool pr, bool strm, float ref, float roll, float max, float vo, float pi, bool _loop);

		inline Sound operator=(const Sound& other) {
			//name = other.name;
			priority = other.priority;
			refDist = other.refDist;
			rolloff = other.rolloff;
			maxDist = other.maxDist;
			vol = other.vol;
			pitch = other.pitch;
			buffer = other.buffer;
			streamed = other.streamed;

			return *this;
		};

		// if not already loaded, load sound now
		void init();

		// play sound; if not already loaded, sound will be loaded first
		// <= sound handle
		unsigned int play(const Ogre::Vector3& = Ogre::Vector3(0,0,0), float customPitch = -1);
		void stopLastPlaying();

		// static functions, wrapping 'SoundBoss' for convenience
		// stop a playing sound
		// => sound handle
		static void stop(unsigned int);
		// is sound playing?
		// => sound handle
		static bool isPlaying(unsigned int);

	};

}
#endif