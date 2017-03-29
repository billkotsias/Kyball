#include "core\Log.h"

#include <stdio.h>
#include <time.h>

namespace FANLib {

	Log::Log(const char* const filename, bool append, bool console, bool stamp, bool internal) : filenameCopy(0), file(0), consoleOutput(console), timeStamp(stamp) {
		if (filename) {

			FastStr fstr(filename, false); /// keep a copy of passed filename
			filenameCopy = fstr.toCString();

			std::ios_base::open_mode openMode = std::ios_base::out; /// define output mode
			if (append) openMode |= std::ios_base::app;

			file = new std::ofstream(filename, openMode);	/// open file
			if (file && !file->is_open()) throw -1;			/// couldn't open file
		}

		/// use first user-created Log instance for internal use
		if (internal && !LOG_INTERNAL) LOG_INTERNAL = this;

		log("--- Log starts ---");
	}

	Log::~Log() {
		log("--- Log ends ---");
		if (filenameCopy) delete [] filenameCopy;
		endLog();
	}

	void Log::endLog() {
		if (file) {
			if (file->is_open()) file->close(); /// remember to destroy Log for this call <-
			delete file;
		}
	}

	Log* Log::LOG_INTERNAL = 0;

	// log FastStr
	void Log::log(const FastStr* fstr, bool nl, int throwExc, bool tStamp) {

		if (tStamp) {
			time(&rawtime);
			localtime_s(&timeinfo, &rawtime);
			strftime(timeBuffer, 18, "%d/%m %H:%M:%S > ", &timeinfo);
		}

		if (consoleOutput) {
			if (tStamp) std::cout << timeBuffer;
			std::cout << *fstr;
			if (nl) std::cout << "\n";
		}

		if (file != 0) {
			if (!file->is_open()) file->open(filenameCopy, std::ios_base::out | std::ios_base::app); /// in case it was closed due to an exception

			if (tStamp) (*file) << timeBuffer;
			(*file) << *fstr;
			if (nl) (*file) << "\n";
		}

		if (throwExc) {
			if (file && file->is_open()) file->close(); /// flush buffers and finalize output file, in case noone catches this exception
			throw throwExc; /// an important error has occured
		}
	}

	// log null-terminated array of 'FastStr's (service function)
	void Log::log(const char* const str[], const FastStr* const fstr[], bool nl, int throwExc) {

		/// 1st log has default timestamp setting
		if (fstr[0]) {
			log(str[0], false, 0);		/// there's another FastStr to go
		} else {
			log(str[0], nl, throwExc);	/// end of logs, produce exception now if desired
			return;
		}

		/// no timestamps from now on
		int ref = 0;

		while (true) {
			if (str[ref+1]) {
				log(fstr[ref], false, 0, false);		/// there's another c-string to go
			} else {
				log(fstr[ref], nl, throwExc, false);	/// end of logs, produce exception now if desired
				return;
			}

			++ref;

			if (fstr[ref]) {
				log(str[ref], false, 0, false);			/// there's another FastStr to go
			} else {
				log(str[ref], nl, throwExc, false);		/// end of logs, produce exception now if desired
				return;
			}
		}
	}

}
