#ifndef __GPUCOMMANDBUFFERFLUSH_H__
#define __GPUCOMMANDBUFFERFLUSH_H__
 
#include "OgrePrerequisites.h"
#include "OgreFrameListener.h"
 
namespace Ogre
{
 
    /** Helper class which can assist you in making sure the -GPU command
        buffer is regularly flushed, so in cases where the -CPU is outpacing the
        -GPU we do not hit a situation where the -CPU suddenly has to stall to 
        wait for more space in the buffer.
    */
	class GpuCommandBufferFlush : public FrameListener, public Ogre::RenderSystem::Listener
    {
    protected:
        bool mUseOcclusionQuery;
        typedef std::vector<HardwareOcclusionQuery*> HOQList;
        HOQList mHOQList;
        size_t mMaxQueuedFrames;
        size_t mCurrentFrame;
        bool mStartPull;
        bool mStarted;

		bool waitingForDeviceRestore;
		bool isDeviceLost;
		bool isRenderSystemSListening;
		bool isFrameListening;

		int calmDownFrames;
		void _stop();
 
    public:

		static const int CALM_DOWN_FRAMES;

        GpuCommandBufferFlush();
        virtual ~GpuCommandBufferFlush();
 
        void start(size_t maxQueuedFrames = 2);
        void stop();
        bool frameStarted(const FrameEvent& evt);
        bool frameEnded(const FrameEvent& evt);
 
		virtual void eventOccurred(const Ogre::String& eventName, const Ogre::NameValuePairList* parameters = 0);
    };
 
}
 
#endif