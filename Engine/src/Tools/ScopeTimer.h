#pragma once

class ScopeTimer {
private:
	std::string m_message;
	bool m_useSeparateFile;
	std::ofstream m_outputStream;
	std::chrono::high_resolution_clock::time_point m_startTime;
public:
	ScopeTimer(const std::string& message, const std::string& outputFile = "");
	~ScopeTimer();

};