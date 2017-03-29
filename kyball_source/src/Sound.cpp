#include "StdAfx.h"
#include "sound\Sound.h"

#include "sound\SoundBoss.h"
#include "P3DScripts.h"
#include "fsl\FSLArray.h"
#include "fsl\FSLInstance.h"
#include "fsl\FSLClassDefinition.h"

namespace P3D {

	std::map<std::string, Sound*> Sound::allSounds;
	Sound* Sound::defaultSound = 0;

	Sound* Sound::BALL_SMALL_POP	= 0;
	Sound* Sound::BALL_BIG_POP		= 0;
	Sound* Sound::BALL_FIRE			= 0;
	Sound* Sound::BALL_BOUNCE		= 0;
	Sound* Sound::BALL_BZZZT		= 0;
	Sound* Sound::GAME_LOSE			= 0;
	Sound* Sound::CHANGE_ENERGY		= 0;
	Sound* Sound::GAME_WIN			= 0;
	Sound* Sound::GAME_WIN_GREAT	= 0;
	Sound* Sound::CUBE_SHRINKING	= 0;
	Sound* Sound::CUBE_SHRINK_IN1	= 0;
	Sound* Sound::CUBE_SHRINK_IN2	= 0;

	void Sound::readScript() {

		/// - parse pre-scripted
		FANLib::FSLArray* sounds = P3DScripts::p3dSound->getRoot()->getArray("sounds");
		for (unsigned int i = 0; i < sounds->getSize(); ++i) {
			FANLib::FSLClass* sound = sounds->getClass(i);
			allSounds[ sound->getCString("name") ] = buildFromFSL(sound);
		}

		/// - parse default sound params (reference for unscripted sounds)
		defaultSound = buildFromFSL( P3DScripts::p3dSound->getClassDef("Sound") );


		/// - "hardcoded" sounds
		Sound::BALL_SMALL_POP	= getScripted("gameplay\\ballSmallPop");
		Sound::BALL_BIG_POP		= getScripted("gameplay\\ballBigPop");
		Sound::BALL_FIRE		= getScripted("gameplay\\ballFire");
		Sound::BALL_BOUNCE		= getScripted("gameplay\\ballBounce");
		Sound::BALL_BZZZT		= getScripted("gameplay\\bzzzt");
		Sound::GAME_LOSE		= getScripted("gameplay\\lose");
		Sound::GAME_WIN			= getScripted("gameplay\\win");
		Sound::GAME_WIN_GREAT	= getScripted("gameplay\\winGreat");
		Sound::CUBE_SHRINKING	= getScripted("gameplay\\shrink");
		Sound::CUBE_SHRINK_IN1	= getScripted("gameplay\\shrinkIn1");
		Sound::CUBE_SHRINK_IN2	= getScripted("gameplay\\shrinkIn2");
		Sound::CHANGE_ENERGY	= getScripted("gameplay\\changeEnergy");
	}

	void Sound::destroyAll() {
		for (std::map<std::string, Sound*>::iterator it = allSounds.begin(); it != allSounds.end(); ++it) {
			delete it->second;
		}
		allSounds.clear();
		delete defaultSound;
		defaultSound = 0;
		Sound::BALL_SMALL_POP	= 0;
		Sound::BALL_BIG_POP		= 0;
		Sound::BALL_FIRE		= 0;
		Sound::BALL_BOUNCE		= 0;
		Sound::BALL_BZZZT		= 0;
		Sound::GAME_LOSE		= 0;
		Sound::GAME_WIN			= 0;
		Sound::GAME_WIN_GREAT	= 0;
		Sound::CUBE_SHRINKING	= 0;
		Sound::CUBE_SHRINK_IN1	= 0;
		Sound::CUBE_SHRINK_IN2	= 0;
		Sound::CHANGE_ENERGY	= 0;
	}

	Sound* Sound::getScripted(const char* const name, const char* const base) {

		std::map<std::string, Sound*>::iterator it = allSounds.lower_bound(name);

		if ( it == allSounds.end() || (allSounds.key_comp()(name, it->first)) ) {
			/// requested sound is <not pre-scripted> and <hasn't been created yet either>, so we'll create one with this name and with default/derived parameters
			Sound* newSound = new Sound(name);
			if (base) {
				(*newSound) = (*getScripted(base));
			} else {
				(*newSound) = (*defaultSound);
			}
			it = allSounds.insert(it, std::map<std::string, Sound*>::value_type(name,newSound) );
		}

		return it->second;
	}

	Sound* Sound::buildFromFSL(FANLib::FSLClass *fsl) {
		//std::cout << "buildFromFSL " << fsl->getCString("name") << " loop=" << (fsl->getInt("loop") != 0) << "\n";
		return new Sound(
			fsl->getCString("name"),
			fsl->getInt("priority") != 0,
			fsl->getInt("streamed") != 0,
			fsl->getReal("refDistance"),
			fsl->getReal("rolloff"),
			fsl->getReal("maxDistance"),
			fsl->getReal("volume"),
			fsl->getReal("pitch"),
			fsl->getInt("loop") != 0
			);
	}

	//

	Sound::Sound(const char* const na, bool pr, bool strm, float ref, float roll, float max, float vo, float pi, bool _loop)
		: name(na), priority(pr), streamed(strm), refDist(ref), rolloff(roll), maxDist(max), vol(vo), pitch(pi), buffer(0), loop(_loop) {
	};

	void Sound::init() {
		if (!streamed) buffer = SoundBoss::getSingletonPtr()->getBuffer(name.c_str());
	}

	unsigned int Sound::play(const Ogre::Vector3& pos, float customPitch)
	{
		if (customPitch < 0) customPitch = pitch; /// use default

		unsigned int voice;

		if (!streamed)
		{
			if (!buffer || !(*buffer)) init();		/// sound not initialized yet; initialize now, but better avoid this practice
			if (!buffer || !(*buffer)) return 0;	/// failed to init (?)
			if (loop)
				voice = SoundBoss::getSingletonPtr()->playAmbience(*buffer, pos, refDist, rolloff, maxDist, vol, customPitch);
			else
				voice = SoundBoss::getSingletonPtr()->playSound(*buffer, priority, pos, refDist, rolloff, maxDist, vol, customPitch);

		} else
		{
			if (loop)
				voice = SoundBoss::getSingletonPtr()->streamAmbience(name, pos, refDist, rolloff, maxDist, vol, customPitch);
			else
				voice = SoundBoss::getSingletonPtr()->streamSound(name, priority, pos, refDist, rolloff, maxDist, vol, customPitch);
		}

		lastPlayingVoice = voice;
		return voice;
	}

	void Sound::stop(unsigned int voice) {
		//std::cout << "Sound::stop\n";
		SoundBoss::getSingletonPtr()->stopSound(voice);
	}

	void Sound::stopLastPlaying(){
		stop(lastPlayingVoice);
		lastPlayingVoice = 0;
	}

	bool Sound::isPlaying(unsigned int voice) {
		return SoundBoss::getSingletonPtr()->isSoundPlaying(voice);
		//return !FreeSL::fslSoundIsInactive(voice);
	}
}