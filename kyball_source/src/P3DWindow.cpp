#include "StdAfx.h"
#include "P3DWindow.h"

#include "P3DApp.h"
#include "P3DConfig.h"
#include "hid/HIDBoss.h"
#include <OgreRoot.h>

#include <core/Utils.h>
#include <string>

#if defined(_WIN32)
#include <windows.h>
#else
#include <SDL.h>
#include <SDL_syswm.h>
#if !defined(__ANDROID__)
#include <X11/Xlib.h>
#endif
#endif

namespace P3D {

	Ogre::RenderWindow* P3DWindow::window = 0;
	int P3DWindow::windowHandle = 0;
	double P3DWindow::pixelRatio = 1.;
	static bool g_closeRequested = false;

	void P3DWindow::requestClose() { g_closeRequested = true; }
	bool P3DWindow::closeRequested() {
		return g_closeRequested || (window && window->isClosed());
	}

#if !defined(_WIN32)
	static SDL_Window* g_sdlWindow = 0;
	static SDL_GLContext g_glContext = 0;

#if !defined(__ANDROID__)
	// Xlib's default handler prints the error and calls exit(). A stale window
	// during close does that, so the menu vanishes with no C++ exception.
	static int logXError(Display* display, XErrorEvent* event) {
		char text[256];
		text[0] = 0;
		if (display && event) XGetErrorText(display, event->error_code, text, sizeof(text));
		std::cerr << "X error (ignored): " << text << std::endl;
		if (event && (event->error_code == BadWindow || event->error_code == BadDrawable))
			g_closeRequested = true;
		return 0;
	}
#endif

	// Ogre renders into SDL's context. SDL has to present that buffer.
	class ExternalBufferSwap : public Ogre::FrameListener {
	public:
		bool frameEnded(const Ogre::FrameEvent&) {
			if (g_sdlWindow) SDL_GL_SwapWindow(g_sdlWindow);
			return true;
		}
	};
#endif

	Ogre::RenderWindow* P3DWindow::create() {

		int windowX = (int)P3DConfig::WIDTH;
		int windowY = (int)P3DConfig::HEIGHT;
		if (windowX < 320) windowX = 1280;
		if (windowY < 240) windowY = 720;
		std::string colourDepth = FANLib::Utils::toString(P3DConfig::DEPTH);
#if defined(_DEBUG) || defined(KYBALL_WINDOWED)
		bool fullScreen = false;
#else
		bool fullScreen = true;
#endif

		Ogre::NameValuePairList list;
		list["FSAA"] = FANLib::Utils::toString(P3DConfig::FSAA);
		list["vsync"] = "true";
		list["Floating-point mode"] = "Consistent";
		list["colourDepth"] = colourDepth;
		list["displayFrequency"] = "60";

#if !defined(_WIN32)
		SDL_DisplayMode desktop;
		if (SDL_GetCurrentDisplayMode(0, &desktop) == 0) {
			if (windowX > desktop.w) windowX = desktop.w;
			if (windowY > desktop.h) windowY = desktop.h;
		}
#if defined(__ANDROID__)
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#endif
		list["externalGLControl"] = "Yes";
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

		Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
		if (fullScreen) flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		g_sdlWindow = SDL_CreateWindow(P3DApp::WINDOW_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowX, windowY, flags);
		if (!g_sdlWindow) {
			OGRE_EXCEPT(Ogre::Exception::ERR_RENDERINGAPI_ERROR, SDL_GetError(), "P3DWindow::create");
		}
		g_glContext = SDL_GL_CreateContext(g_sdlWindow);
		if (!g_glContext) {
			OGRE_EXCEPT(Ogre::Exception::ERR_RENDERINGAPI_ERROR, SDL_GetError(), "P3DWindow::create");
		}
		SDL_GL_MakeCurrent(g_sdlWindow, g_glContext);
		SDL_GL_SetSwapInterval(1);
		list["currentGLContext"] = "Yes";
		HIDBoss::setSdlWindow(g_sdlWindow);

		SDL_SysWMinfo info;
		SDL_VERSION(&info.version);
		if (SDL_GetWindowWMInfo(g_sdlWindow, &info)) {
#if defined(__ANDROID__)
			windowHandle = 0;
#else
			windowHandle = (int)info.info.x11.window;
#endif
		}
		static ExternalBufferSwap swapper;
		Ogre::Root::getSingletonPtr()->addFrameListener(&swapper);
		// SDL owns the real window; Ogre renders into the current GL context.
		setWindow(Ogre::Root::getSingletonPtr()->createRenderWindow(P3DApp::WINDOW_NAME, windowX, windowY, false, &list));
#if !defined(__ANDROID__)
		XSetErrorHandler(logXError);
#endif
		if (fullScreen) {
			pixelRatio = P3DConfig::MONITOR_ASPECT_RATIO * window->getHeight() / window->getWidth();
		} else {
			pixelRatio = 1.;
		}
#else
		setWindow( Ogre::Root::getSingletonPtr()->createRenderWindow(P3DApp::WINDOW_NAME, windowX, windowY, fullScreen, &list) );
#endif
		return window;
	}

	void P3DWindow::setWindow(Ogre::RenderWindow* win) {
		window = win;
		window->setDeactivateOnFocusChange(false);
#if defined(_WIN32)
		window->getCustomAttribute("WINDOW", &windowHandle);
#endif
		if (window->isFullScreen()) {
			pixelRatio = P3DConfig::MONITOR_ASPECT_RATIO * window->getHeight() / window->getWidth();
		} else {
			pixelRatio = 1.;
		}
	}

	int P3DWindow::getWindowHandle() {
		return windowHandle;
	}

	bool P3DWindow::focused() {
#if defined(_WIN32)
		return ((int)GetForegroundWindow() == (int)windowHandle);
#else
		return HIDBoss::applicationFocused();
#endif
	}

	Ogre::Vector2 P3DWindow::getSize() {
		static Ogre::Vector2 size(window->getWidth(), window->getHeight());
		return size;
	}

	double P3DWindow::getWindowRatio() {
		static double ratio = (double)window->getWidth() / window->getHeight();
		return ratio;
	}
}
