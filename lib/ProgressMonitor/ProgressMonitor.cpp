// Includes
// ==========
// STL includes
#include <vector>
#include <ctime>
#include <chrono>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>

// Program includes
#include "progressmonitor.h"

// OS-dependent includes
#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

/**
 * Get the current UTC time in seconds depending on the operating system
 *
 * @param struct *ts Specification of current time in seconds (ts.tv_sec) and remaining nanoseconds (ts.tv_nsec)
 */
void current_utc_time( struct timespec *ts ) {

#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
	clock_serv_t cclock;
	mach_timespec_t mts;
	host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
	clock_get_time(cclock, &mts);
	mach_port_deallocate(mach_task_self(), cclock);
	ts->tv_sec = mts.tv_sec;
	ts->tv_nsec = mts.tv_nsec;
#else
	clock_gettime(CLOCK_REALTIME, ts);
#endif
}

ProgressMonitor::ProgressMonitor() {
	reset();
}

double ProgressMonitor::getCurrentClockTime() {
	struct timespec ts;
	current_utc_time( &ts );

	return ts.tv_sec + ts.tv_nsec / 1000000000.0;
}

void ProgressMonitor::reset() {
	laps.clear();
	start = getCurrentClockTime();
}

double ProgressMonitor::lap() {
	double currenttime = getCurrentClockTime();

	laps.push_back( currenttime );

	if ( laps.size() == 1 ) {
		return currenttime - start;
	}
	else {
		return currenttime - laps[ laps.size() - 2 ];
	}
}

double ProgressMonitor::stop() {
	end = getCurrentClockTime();

	return end - start;
}