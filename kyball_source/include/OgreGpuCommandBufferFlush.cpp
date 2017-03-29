#include "Stdafx.h"
#include "OgreGpuCommandBufferFlush.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreHardwareOcclusionQuery.h"
#include "core\Utils.h"

namespace Ogre
{
	const int GpuCommandBufferFlush::CALM_DOWN_FRAMES = 1;

    //---------------------------------------------------------------------
    GpuCommandBufferFlush::GpuCommandBufferFlush()
        : mUseOcclusionQuery(false)
        , mMaxQueuedFrames(2)
        , mCurrentFrame(0)
        , mStartPull(false)
        , mStarted(false)
		, waitingForDeviceRestore(false)
		, isDeviceLost(false)
		, isRenderSystemSListening(false)
		, isFrameListening(false)
    {
 
    }
    //---------------------------------------------------------------------
    GpuCommandBufferFlush::~GpuCommandBufferFlush()
    {
        stop();
		RenderSystem* rsys;
		Ogre::Root* root;
		if ( (root = Root::getSingletonPtr()) && (rsys = root->getRenderSystem()) ) {
			if (isRenderSystemSListening) rsys->removeListener(this);
			if (isFrameListening) root->removeFrameListener(this);
		}
    }
    //---------------------------------------------------------------------
	void GpuCommandBufferFlush::eventOccurred(const Ogre::String& eventName, const Ogre::NameValuePairList* parameters)
	{
		Ogre::LogManager::getSingletonPtr()->logMessage(std::string("GpuCommandBufferFlush eventOccurred=") + eventName);
		if (eventName == "DeviceLost") {
			isDeviceLost = true;
			Ogre::LogManager::getSingletonPtr()->logMessage(FANLib::Utils::toString(mStarted) + std::string("/") + FANLib::Utils::toString(waitingForDeviceRestore));
			if (mStarted) waitingForDeviceRestore = true; /// we were started and device was frigging lost!!!
			_stop();
		} else if (eventName == "DeviceRestored") {
			isDeviceLost = false;
			//start();
			//if (waitingForDeviceRestore) {
			//	Ogre::LogManager::getSingletonPtr()->logMessage(std::string("DEVICE FINALLY RESTORED!!!\n"));
			//	start();
			//}
		}
	}
    //---------------------------------------------------------------------
    void GpuCommandBufferFlush::start(size_t maxQueuedFrames)
    {
		return;
		Ogre::LogManager::getSingletonPtr()->logMessage(std::string("BILL START CALLED\n"));
		if (mStarted || isDeviceLost) return;

		Ogre::LogManager::getSingletonPtr()->logMessage(std::string("BILL START CALLED1\n"));

        _stop();

		RenderSystem* rsys;
		Ogre::Root* root;
		if ( !(root = Root::getSingletonPtr()) || !(rsys = root->getRenderSystem()) )
            return;
 
        if (!mUseOcclusionQuery) mUseOcclusionQuery = rsys->getCapabilities()->hasCapability(RSC_HWOCCLUSION);
		if (!mUseOcclusionQuery) return; // stop now

		for (size_t i = 0; i < mMaxQueuedFrames; ++i)
		{
			HardwareOcclusionQuery* hoq = rsys->createHardwareOcclusionQuery();
			mHOQList.push_back(hoq);
		}
        mMaxQueuedFrames = maxQueuedFrames;
		mCurrentFrame = 0;
		mStartPull = false;
		mStarted = true;
 
		if (!isRenderSystemSListening) {
			rsys->addListener(this);
			isRenderSystemSListening = true;
		}
		if (!isFrameListening) {
			root->addFrameListener(this);
			isFrameListening = true;
		}

		Ogre::LogManager::getSingletonPtr()->logMessage(std::string("BILL MSTARTED!!!\n"));
    }
    //---------------------------------------------------------------------
    void GpuCommandBufferFlush::stop()
    {
		waitingForDeviceRestore = false;
		_stop();
	}

    void GpuCommandBufferFlush::_stop()
    {
		Ogre::LogManager::getSingletonPtr()->logMessage(std::string("BILL STOP CALLED"));
        mStarted = false;

		RenderSystem* rsys;
		Ogre::Root* root;
		if ( !(root = Root::getSingletonPtr()) || !(rsys = root->getRenderSystem()) )
            return;

		if (isFrameListening) {
			root->removeFrameListener(this);
			isFrameListening = false;
		}

        for (HOQList::iterator i = mHOQList.begin(); i != mHOQList.end(); ++i) {
            rsys->destroyHardwareOcclusionQuery(*i);
        }
        mHOQList.clear();

		Ogre::LogManager::getSingletonPtr()->logMessage(std::string("BILL STOP ENDED\n"));
    }
    //---------------------------------------------------------------------
    bool GpuCommandBufferFlush::frameStarted(const FrameEvent& evt)
    {
		if (!mStarted) return true;
		//Ogre::LogManager::getSingletonPtr()->logMessage(std::string("BILL frameStarted"));

        mHOQList[mCurrentFrame]->beginOcclusionQuery();
        return true;
    }
    //---------------------------------------------------------------------
    bool GpuCommandBufferFlush::frameEnded(const FrameEvent& evt)
    {
		if (!mStarted) return true;
		//Ogre::LogManager::getSingletonPtr()->logMessage(std::string("BILL frameEnded"));

		if(mHOQList[mCurrentFrame]->isStillOutstanding())
            mHOQList[mCurrentFrame]->endOcclusionQuery();

        mCurrentFrame = (mCurrentFrame + 1) % mMaxQueuedFrames;
		if (mStartPull) {
            unsigned int dummy;
            mHOQList[mCurrentFrame]->pullOcclusionQuery(&dummy);
		} else {
			if (mCurrentFrame == 0) mStartPull = true; // If we've wrapped around, time to start pulling
		}
 
        return true;
    }
    //---------------------------------------------------------------------
 
}