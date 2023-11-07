#include "pch.h"
#include "Tools/Profiler.h"
#include "Tools/Logger.hpp"

namespace sa {


	Profiler::Profiler()
		: m_profileCount(0)
	{
	}

	Profiler::~Profiler() {
		endSession();
	}

	void Profiler::beginSession(const std::string& filepath) {
		SA_DEBUG_LOG_INFO("PROFILER: Starting session..., File: ", filepath);
		if (m_outputStream.is_open())
			return;

		m_mutex.lock();

		m_outputStream.open(filepath);
		if (!m_outputStream.is_open())
		{
			SA_DEBUG_LOG_ERROR("Failed to open session file");
			m_mutex.unlock();
			return;
		}
		writeHeader();

		m_mutex.unlock();
	}

	void Profiler::endSession()
	{
		SA_DEBUG_LOG_INFO("PROFILER: Ending session...");

		if (!m_outputStream.is_open())
			return;

		m_mutex.lock();

		writeFooter();
		m_outputStream.close();
		m_profileCount = 0;

		m_mutex.unlock();
	}

	void Profiler::writeHeader()
	{
		m_outputStream << "{\"otherData\": {}, \"traceEvents\":[";
		m_outputStream.flush();
	}

	void Profiler::writeProfile(const Profiler::Result& result)
	{
		if (!m_outputStream.is_open())
			return;

		m_mutex.lock();

		if (m_profileCount++ > 0)
		{
			m_outputStream << ",";
		}

		std::string name = result.name;
		std::replace(name.begin(), name.end(), '"', '\'');

		m_outputStream << "{";
		m_outputStream << "\"cat\":\"function\",";
		m_outputStream << "\"dur\":" << (result.end - result.start) << ',';
		m_outputStream << "\"name\":\"" << name << "\",";
		m_outputStream << "\"ph\":\"X\",";
		m_outputStream << "\"pid\":0,";
		m_outputStream << "\"tid\":" << result.threadID << ",";
		m_outputStream << "\"ts\":" << result.start;
		m_outputStream << "}";

		m_outputStream.flush();

		m_mutex.unlock();
	}

	void Profiler::writeFooter()
	{
		m_outputStream << "]}";
		m_outputStream.flush();
	}

	Profiler::ProfileTimer::ProfileTimer(const std::string& name)
		: m_name(name)
	{
	}

	Profiler::ProfileTimer::~ProfileTimer() {
		Profiler::Result res;
		res.start = m_clock.getStartTime<std::chrono::microseconds>();
		res.end = res.start + m_clock.getElapsedTime<std::chrono::microseconds>();
		res.name = m_name;
		res.threadID = uint32_t(std::hash<std::thread::id>()(std::this_thread::get_id()));
		Profiler::get().writeProfile(res);
	}

}