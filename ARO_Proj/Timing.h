////////////////////
// Timing.h - header file for IntervalTimer, StopwatchTimer, and TimeStampGenerator
// Last edited: 08/26/2021 by Andrew O'Kins
////////////////////

#ifndef TIMING_H_
#define TIMING_H_

#include <Windows.h>

// This timer gives how much time has elapsed since the generator's construction
// used in the optimization for both timing output and in checking against stop/timeout conditions
class TimeStampGenerator {
private:
	__int64 frequency_;
	__int64 start_time_;
public:
	//constructor
	TimeStampGenerator() {
		QueryPerformanceCounter((LARGE_INTEGER *)&start_time_);
		QueryPerformanceFrequency((LARGE_INTEGER *)&frequency_);
	}
	// Return number of seconds that have passed since the generator has been constructed
	double S_SinceStart() {
		__int64 currentTime;
		QueryPerformanceCounter((LARGE_INTEGER *)&currentTime);
		double seconds = double((currentTime - start_time_) / frequency_);
		return seconds;
	}
	// Return number of milliseconds that have passed since the generator has been constructed
	double MS_SinceStart() {
		__int64 currentTime;
		QueryPerformanceCounter((LARGE_INTEGER *)&currentTime);
		double milliseconds = double((currentTime - start_time_) / (frequency_ / 1000));
		return milliseconds;
	}

	// Return number of microseconds that have passed since the generator has been constructed
	double MicroS_SinceStart() {
		__int64 currentTime;
		QueryPerformanceCounter((LARGE_INTEGER *)&currentTime);
		double microseconds = double((currentTime - start_time_) / (frequency_ / 1000000));
		return microseconds;
	}
};
#endif
