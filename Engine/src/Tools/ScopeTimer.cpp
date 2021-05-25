#include "Saturn/Tools/ScopeTimer.h"

ScopeTimer::ScopeTimer(const std::string& message, const std::string& outputFile)
	: m_message(message)
{
	if (outputFile.length() != 0) {
		m_useSeparateFile = true;
		m_outputStream = std::ofstream(outputFile, std::ios::app);
		if (!m_outputStream.is_open()) {
			m_useSeparateFile = false;
		}
	}
	else {
		m_useSeparateFile = false;
	}

	m_startTime = std::chrono::high_resolution_clock::now();
}

ScopeTimer::~ScopeTimer()
{
	using namespace std::chrono_literals;

	auto timeNano = std::chrono::high_resolution_clock::now() - m_startTime;
	double time = timeNano.count();
	std::string timeLiteral = " ns";
	if (timeNano > 1min) {
		auto timeMin = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<60, 1>>, double, std::nano>(timeNano);
		time = timeMin.count();
		timeLiteral = " min";
	}
	else if (timeNano > 1s) {
		auto timeSec = std::chrono::duration_cast<std::chrono::duration<double>, double, std::nano>(timeNano);
		time = timeSec.count();
		timeLiteral = " s";
	}
	else if (timeNano > 1ms) {
		auto timeMilli = std::chrono::duration_cast<std::chrono::duration<double, std::milli>, double, std::nano>(timeNano);
		time = timeMilli.count();
		timeLiteral = " ms";
	}

	if (m_useSeparateFile) {
		m_outputStream << "{ ScopeTimer } " << m_message << "\t" << time << timeLiteral << std::endl;
		m_outputStream.close();
	}
	else {
		std::cout << "{ ScopeTimer } " << m_message << "\t" << time << timeLiteral << std::endl;
	}
}
