// Fantasia Scripting Language - Enumeration instance

#pragma once
#ifndef FANLIB_HRTimer_H
#define FANLIB_HRTimer_H

#if defined(_WIN32)
#include <Windows.h>
#else
#include <stdint.h>
#endif

namespace FANLib {

	class HRTimer {

	public:

		HRTimer(void);

		double getFrequency(void);
		void startTimer(void);
		double stopTimer(void);

	private:

#if defined(_WIN32)
		LARGE_INTEGER start;
		LARGE_INTEGER stop;
#else
		int64_t start;
		int64_t stop;
#endif

		double frequency;

	};

}

#endif
