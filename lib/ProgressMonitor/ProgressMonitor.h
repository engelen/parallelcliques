#ifndef PROGRESSMONITOR_H
#define PROGRESSMONITOR_H

// Includes
// ==========
// STL includes
#include <vector>

class ProgressMonitor {
	private:
		/**
		 * Time (in seconds) the monitor started recording. Can be reset using reset().
		 */
		double start = 0;

		/**
		 * Time (in seconds) the monitor ended recording.
		 */
		double end = 0;

		/**
		 * List of lap times, used to keep track of time recordings between start and end. Start and end
		 * do not count as laps themselves.
		 */
		std::vector<double> laps;

	public:
		/**
		 * Constructor. Resets monitor.
		 */
		ProgressMonitor();

		/**
		 * Reset the monitor by setting the start time to the current time and clearing the list of laps.
		 */
		void reset();

		/**
		 * Record the current time as a lap and return the difference between this time and the previous lap
		 * time. If this is the first lap, return the time between the current time and the start time.
		 *
		 * @return double Lap time
		 */
		double lap();

		/**
		 * Stop the timer, recording the current time as the stop time. Returns the elapsed time between the
		 * stop time and the start time.
		 *
		 * @return double Time elapses between stop time and start time
		 */
		double stop();

		/**
		 * Get the current clock time in seconds.
		 *
		 * @return double Current clock time
		 */
		double getCurrentClockTime();
};

#endif
