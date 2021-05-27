#pragma once
#include <chrono>

class StopWatch {
private:
	int m_laps;
	std::chrono::high_resolution_clock::time_point m_startTime;
public:
	StopWatch();

	void start();
	void stop();

	std::chrono::duration<double, std::milli> getDuration();
	std::chrono::duration<double, std::milli> getAverageLapDuration();
};