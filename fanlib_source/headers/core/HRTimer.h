// Fantasia Scripting Language - Enumeration instance

#pragma once
#ifndef FANLIB_HRTimer_H
#define FANLIB_HRTimer_H

#include <Windows.h>

namespace FANLib {

	class HRTimer {

	public:

		HRTimer(void);

		double getFrequency(void);
		void startTimer(void) ;
		double stopTimer(void);

	private:

		LARGE_INTEGER start;
		LARGE_INTEGER stop;

		double frequency;

	};

}

#endif