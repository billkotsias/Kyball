#include "KyballPlatform.h"
#include "P3DApp.h"

#include <OgreRoot.h>

#include <fstream>
#include <iostream>
#include <vector>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#include <direct.h>
#endif

#if defined(__ANDROID__)
#include <SDL.h>
#include <jni.h>
#include <zlib.h>
#include <stdint.h>
#include <OgreGLES2Plugin.h>
#include <OgreParticleFXPlugin.h>
#endif

#ifndef KYBALL_PLUGIN_DIR
#define KYBALL_PLUGIN_DIR "."
#endif

namespace P3D {

	void makeDirectory(const std::string& path) {
		if (path.empty()) return;
		std::string cur;
		cur.reserve(path.size());
		for (size_t i = 0; i < path.size(); ++i) {
			char ch = path[i];
			cur.push_back(ch);
			if (ch == '/' || ch == '\\') {
#if defined(_WIN32)
				_mkdir(cur.c_str());
#else
				if (mkdir(cur.c_str(), 0755) != 0 && errno != EEXIST) { }
#endif
			}
		}
#if defined(_WIN32)
		_mkdir(path.c_str());
#else
		if (mkdir(path.c_str(), 0755) != 0 && errno != EEXIST) { }
#endif
	}

	void reportError(const char* message) {
#if defined(_WIN32)
		MessageBoxA(NULL, message ? message : "", "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
		std::cerr << "An exception has occured: " << (message ? message : "") << std::endl;
#endif
	}

	static void writePluginsFile() {
#if defined(KYBALL_STATIC_PLUGINS)
		return;
#else
		std::ifstream existing(P3DApp::PLUGINS_FILE.c_str());
		if (existing.good()) return;
		makeDirectory(P3DApp::DATA_PATH);
		std::ofstream out(P3DApp::PLUGINS_FILE.c_str());
		if (!out) return;
		out << "# Generated for Ogre 1.10\n";
		out << "PluginFolder=" << KYBALL_PLUGIN_DIR << "\n";
#if defined(__ANDROID__)
		out << "Plugin=RenderSystem_GLES2\n";
#else
		out << "Plugin=RenderSystem_GL\n";
#endif
		out << "Plugin=Plugin_ParticleFX\n";
#endif
	}

#if defined(__ANDROID__)
	static uint16_t ru16(const unsigned char* p) {
		return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
	}
	static uint32_t ru32(const unsigned char* p) {
		return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
	}

	// Android's asset directory listing hides subfolders, so copy P3DData out of the APK zip.
	static void extractPackagedData() {
		JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
		jobject activity = (jobject)SDL_AndroidGetActivity();
		if (!env || !activity) return;
		jclass activityCls = env->GetObjectClass(activity);
		jmethodID getInfo = env->GetMethodID(activityCls, "getApplicationInfo", "()Landroid/content/pm/ApplicationInfo;");
		jobject info = getInfo ? env->CallObjectMethod(activity, getInfo) : 0;
		std::string apkPath;
		if (info) {
			jclass infoCls = env->GetObjectClass(info);
			jfieldID sourceDir = env->GetFieldID(infoCls, "sourceDir", "Ljava/lang/String;");
			jstring jpath = sourceDir ? (jstring)env->GetObjectField(info, sourceDir) : 0;
			if (jpath) {
				const char* chars = env->GetStringUTFChars(jpath, 0);
				if (chars) apkPath = chars;
				env->ReleaseStringUTFChars(jpath, chars);
				env->DeleteLocalRef(jpath);
			}
			env->DeleteLocalRef(infoCls);
			env->DeleteLocalRef(info);
		}
		env->DeleteLocalRef(activityCls);
		env->DeleteLocalRef(activity);
		if (apkPath.empty()) return;

		std::ifstream in(apkPath.c_str(), std::ios::binary);
		if (!in) return;
		in.seekg(0, std::ios::end);
		std::streamoff length = in.tellg();
		if (length < 22) return;
		std::vector<unsigned char> zip((size_t)length);
		in.seekg(0, std::ios::beg);
		in.read(reinterpret_cast<char*>(&zip[0]), length);

		const std::string prefix = "assets/P3DData/";
		size_t scanStart = zip.size() > 66000 ? zip.size() - 66000 : 0;
		size_t eocd = (size_t)-1;
		for (size_t i = zip.size() - 22; ; --i) {
			if (i < scanStart) break;
			if (ru32(&zip[i]) == 0x06054b50u) { eocd = i; break; }
			if (i == 0) break;
		}
		if (eocd == (size_t)-1) return;
		uint32_t entries = ru16(&zip[eocd + 10]);
		uint32_t cdOff = ru32(&zip[eocd + 16]);
		size_t p = cdOff;
		for (uint32_t n = 0; n < entries; ++n) {
			if (p + 46 > zip.size() || ru32(&zip[p]) != 0x02014b50u) break;
			uint16_t method = ru16(&zip[p + 10]);
			uint32_t compSize = ru32(&zip[p + 20]);
			uint32_t uncompSize = ru32(&zip[p + 24]);
			uint16_t nameLen = ru16(&zip[p + 28]);
			uint16_t extraLen = ru16(&zip[p + 30]);
			uint16_t commentLen = ru16(&zip[p + 32]);
			uint32_t localOff = ru32(&zip[p + 42]);
			if (p + 46 + nameLen > zip.size()) break;
			std::string entry(reinterpret_cast<const char*>(&zip[p + 46]), nameLen);
			p += 46 + nameLen + extraLen + commentLen;
			if (entry.compare(0, prefix.size(), prefix) != 0) continue;
			if (!entry.empty() && entry[entry.size() - 1] == '/') {
				makeDirectory(P3DApp::DATA_PATH + entry.substr(prefix.size()));
				continue;
			}
			if ((size_t)localOff + 30 > zip.size()) continue;
			uint16_t localName = ru16(&zip[localOff + 26]);
			uint16_t localExtra = ru16(&zip[localOff + 28]);
			size_t dataOff = (size_t)localOff + 30 + localName + localExtra;
			if (dataOff + compSize > zip.size()) continue;
			std::vector<unsigned char> raw;
			if (method == 0) {
				raw.assign(zip.begin() + dataOff, zip.begin() + dataOff + compSize);
			} else if (method == 8) {
				raw.resize(uncompSize ? uncompSize : 1);
				z_stream strm;
				memset(&strm, 0, sizeof(strm));
				strm.next_in = const_cast<Bytef*>(&zip[dataOff]);
				strm.avail_in = compSize;
				strm.next_out = &raw[0];
				strm.avail_out = (uInt)raw.size();
				if (inflateInit2(&strm, -MAX_WBITS) != Z_OK) continue;
				int result = inflate(&strm, Z_FINISH);
				inflateEnd(&strm);
				if (result != Z_STREAM_END) continue;
				raw.resize(strm.total_out);
			} else {
				continue;
			}
			std::string dest = P3DApp::DATA_PATH + entry.substr(prefix.size());
			std::string parent = dest.substr(0, dest.find_last_of('/'));
			makeDirectory(parent);
			std::ofstream out(dest.c_str(), std::ios::binary);
			if (out && !raw.empty()) out.write(reinterpret_cast<char*>(&raw[0]), (std::streamsize)raw.size());
		}
	}
#endif

	void P3DApp::initPaths() {
#if defined(_WIN32)
		PATH_DELIMITER = "\\";
		const char* appdata = std::getenv("APPDATA");
		USER_PATH = std::string(appdata ? appdata : ".") + PATH_DELIMITER;
#if defined(_DEBUG) && !defined(_DEPLOY)
		DATA_PATH = std::string("..\\P3DData") + PATH_DELIMITER;
#else
		DATA_PATH = std::string("P3DData") + PATH_DELIMITER;
#endif
		PLUGINS_FILE = "P3DData\\plugins.cfg";
#else
		PATH_DELIMITER = "/";
		const char* xdg = std::getenv("XDG_DATA_HOME");
		const char* home = std::getenv("HOME");
		if (xdg && xdg[0]) USER_PATH = std::string(xdg) + "/";
		else if (home && home[0]) USER_PATH = std::string(home) + "/.local/share/";
		else USER_PATH = "./";
#if defined(__ANDROID__)
		const char* internal = SDL_AndroidGetInternalStoragePath();
		if (internal && internal[0]) USER_PATH = std::string(internal) + "/";
#endif
		const char* dataEnv = std::getenv("KYBALL_DATA");
		if (dataEnv && dataEnv[0]) {
			DATA_PATH = dataEnv;
			if (DATA_PATH[DATA_PATH.size() - 1] != '/') DATA_PATH += "/";
		} else {
#if defined(__ANDROID__)
			DATA_PATH = USER_PATH + "P3DData/";
#else
			DATA_PATH = "P3DData/";
#endif
		}
		PLUGINS_FILE = DATA_PATH + "plugins.cfg";
#endif
#ifdef _DEMO
		APP_FOLDER = USER_PATH + "KYBALL_DEMO" + PATH_DELIMITER;
#else
		APP_FOLDER = USER_PATH + "KYBALL" + PATH_DELIMITER;
#endif
		OGRE_CONFIG_FILE = APP_FOLDER + "ogre.cfg";
		OGRE_LOG_FILE = APP_FOLDER + "ogre.log";
		FANLIB_LOG_FILE = APP_FOLDER + "fanlib.log";
		MYGUI_LOG_FILE = APP_FOLDER + "mygui.log";
		RESOURCES_FILE = DATA_PATH + "resources.cfg";
		SCRIPTS_PATH = DATA_PATH + "scripts" + PATH_DELIMITER;
		SOUNDS_PATH = DATA_PATH + "snd" + PATH_DELIMITER;
		OPTIONS_FILE = APP_FOLDER + "options2.txt";
#if defined(__ANDROID__)
		extractPackagedData();
#endif
	}

	Ogre::Root* createOgreRoot() {
		P3DApp::initPaths();
		makeDirectory(P3DApp::APP_FOLDER);
		makeDirectory(P3DApp::DATA_PATH);
		writePluginsFile();
#if defined(KYBALL_STATIC_PLUGINS)
		Ogre::Root* root = OGRE_NEW Ogre::Root("", P3DApp::OGRE_CONFIG_FILE, P3DApp::OGRE_LOG_FILE);
		static Ogre::GLES2Plugin* gles2 = new Ogre::GLES2Plugin();
		static Ogre::ParticleFXPlugin* particles = new Ogre::ParticleFXPlugin();
		root->installPlugin(gles2);
		root->installPlugin(particles);
		return root;
#else
		return OGRE_NEW Ogre::Root(P3DApp::PLUGINS_FILE, P3DApp::OGRE_CONFIG_FILE, P3DApp::OGRE_LOG_FILE);
#endif
	}

}
