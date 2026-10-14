#include "core/HRTimer.h"

#if !defined(_WIN32)
#include <time.h>
#endif

namespace FANLib {

	HRTimer::HRTimer(void) {
		frequency = 1.0 / this->getFrequency();
	}

#if defined(_WIN32)

	double HRTimer::getFrequency(void)
	{

		LARGE_INTEGER proc_freq;

		if (!::QueryPerformanceFrequency(&proc_freq)) throw "QueryPerformanceFrequency() failed";

		return (double)proc_freq.QuadPart;

	}

	void HRTimer::startTimer(void)
	{

		DWORD_PTR oldmask = ::SetThreadAffinityMask(::GetCurrentThread(), 0);
		::QueryPerformanceCounter(&start);
		::SetThreadAffinityMask(::GetCurrentThread(), oldmask);

	}

	double HRTimer::stopTimer(void)
	{

		DWORD_PTR oldmask = ::SetThreadAffinityMask(::GetCurrentThread(), 0);
		::QueryPerformanceCounter(&stop);
		::SetThreadAffinityMask(::GetCurrentThread(), oldmask);

		return ((stop.QuadPart - start.QuadPart) * frequency);

	}

#else

	double HRTimer::getFrequency(void) {
		return 1000000000.0;
	}

	static int64_t readClock() {
		timespec ts;
		clock_gettime(CLOCK_MONOTONIC, &ts);
		return (int64_t)ts.tv_sec * 1000000000LL + (int64_t)ts.tv_nsec;
	}

	void HRTimer::startTimer(void) {
		start = readClock();
	}

	double HRTimer::stopTimer(void) {
		stop = readClock();
		return (double)(stop - start) * frequency;
	}

#endif

}
