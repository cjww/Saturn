#pragma once
#include <chrono>
#include <fstream>
#include <iostream>

class ScopeTimer {
	std::chrono::high_resolution_clock::time_point m_startTime;
	std::ofstream m_outputStream;
	bool m_useSeparateFile;
	std::string m_message;
public:
	ScopeTimer(const std::string& message = "", const std::string& outputFile = "");
	~ScopeTimer();
};

