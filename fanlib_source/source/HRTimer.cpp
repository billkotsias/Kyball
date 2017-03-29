#include "core\HRTimer.h"

namespace FANLib {

	HRTimer::HRTimer(void) {
		frequency = 1.0 / this->getFrequency();
	}

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

}