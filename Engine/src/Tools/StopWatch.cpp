#include "StopWatch.h"

StopWatch::StopWatch() 
	: m_laps(0)
{

}

void StopWatch::start() {
	m_startTime = std::chrono::high_resolution_clock::now();
	m_laps++;
}

std::chrono::duration<double, std::milli> StopWatch::getDuration() {
	auto now = std::chrono::high_resolution_clock::now();
	return now - m_startTime;
}

std::chrono::duration<double, std::milli> StopWatch::getAverageLapDuration()
{
	return getDuration() / m_laps;
}
