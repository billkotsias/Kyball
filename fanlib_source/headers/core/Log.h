// Log : keep log easily
#pragma once
#ifndef FANLIB_Log_H
#define FANLIB_Log_H

#include <fstream>
#include <core\FastStr.h>

namespace FANLib {

	class FastStr;

	class Log {
	private:

		std::ofstream* file;	/// although a pointer, it's a complicated cast 8-) so <fstream> has to be included
		char* filenameCopy;		/// keep a copy of filename, in case we have to re-open it
		time_t rawtime;			/// time related stuff
		struct tm timeinfo;
		char timeBuffer[80];

		static Log* LOG_INTERNAL; /// internal Log
		void endLog(); /// used by destructor

	public:

		// constructor
		// => filename	= file for log output
		//	  append	= append to file (true) or overwrite?
		//	  console	= output log to console window?
		//	  stamp		= add timestamp at start of string?
		//	  internal	= enable internal log?
		Log(const char* const = 0, bool = false, bool = true, bool = true, bool = false);
		
		// destructor
		~Log();

		bool fileOutput;
		bool consoleOutput;
		bool timeStamp; /// add timestamp at start of string?

		// log text
		// => fstr	= pointer to FastStr
		//	  nl	= add newline at end of string?
		//	  throwExc = throw exception because an important error has occured (default: 0 = don't throw)
		//	  stamp = override default timestamp setting
		void log(const FastStr*, bool, int, bool);

		/// => short version : fstr, nl = true, throwExc = 0, stamp timeStamp
		inline void Log::log(const FastStr* const fstr, bool nl = true, int throwExc = 0) {
			log(fstr, nl, throwExc, timeStamp);
		};
		/// => char* version
		inline void Log::log(const char* const str, bool nl, int throwExc, bool tStamp) {
			log(&FastStr(str, false), nl, throwExc, tStamp);
		};
		/// => short char* version
		inline void Log::log(const char* const str, bool nl = true, int throwExc = 0) {
			log(str, nl, throwExc, timeStamp);
		};

		// special log function for dynamic message logs
		// => str	= array of null-terminated c-string pointers
		//	  fstr	= array of null-terminated FastStr pointers
		//	  nl	= add newline at end of string?
		//	  throwExc = throw exception because an important error has occured (default: 0 = don't throw)
		void log(const char* const [], const FastStr* const [], bool = true, int = 0);

		// for internal FANLib usage
		inline static void Log::internalLog(const FastStr* const fstr, bool nl = true, int throwExc = 0, int stamp = 0) {
			if (!LOG_INTERNAL) {
				/// internal Log not enabled : at least report exception if present
				if (throwExc) throw throwExc;
				return;
			}

			bool tStamp = (stamp) ? (stamp > 0 ? true : false) : LOG_INTERNAL->timeStamp;
			LOG_INTERNAL->log(fstr, nl, throwExc, tStamp);
		}
		inline static void Log::internalLog(const char* const str, bool nl = true, int throwExc = 0, int stamp = 0) {
			Log::internalLog(&FastStr(str), nl, throwExc, stamp);
		}
		inline static void Log::internalLog(const char* const str[], const FastStr* fstr[], bool nl = true, int throwExc = 0) {
			if (!LOG_INTERNAL) {
				/// internal Log not enabled : at least report exception if present
				if (throwExc) throw throwExc;
				return;
			}

			LOG_INTERNAL->log(str, fstr, nl, throwExc);
		}
	};
}

#endif // FANLIB_Log_H