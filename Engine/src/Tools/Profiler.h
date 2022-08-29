#pragma once
#include "Clock.h"
#include <fstream>
#include <mutex>
#include <string>
namespace sa {

	// Define SA_PROFILER_ENABLE to enable
	class Profiler 
	{
	private:
		std::ofstream m_outputStream;
		int m_profileCount;

		std::mutex m_mutex;
	public:
		struct Result
		{
			std::string name;
			long long start, end;
			uint32_t threadID;
			Result(): start(0), end(0), threadID(0){} ;
		};

		class ProfileTimer
		{
		private:
			Clock m_clock;
			std::string m_name;
		public:
			ProfileTimer(const std::string& name);
			~ProfileTimer();

		};

		Profiler();

		static Profiler& get()
		{
			static Profiler instance;
			return instance;
		}

		void beginSession(const std::string& filepath = "profile_result.json");

		void endSession();

		void writeHeader();
	
		void writeProfile(const Profiler::Result& result);
	
		void writeFooter();


	};

}


#ifdef SA_PROFILER_ENABLE
#define SA_PROFILER_BEGIN_SESSION_PATH(filepath) sa::Profiler::get().beginSession(filepath)
#define SA_PROFILER_BEGIN_SESSION() sa::Profiler::get().beginSession()
#define SA_PROFILER_END_SESSION() sa::Profiler::get().endSession()
#define SA_PROFILE_SCOPE(name) sa::Profiler::ProfileTimer timer##__LINE__(name)
#define SA_PROFILE_FUNCTION() SA_PROFILE_SCOPE(__FUNCSIG__)
#else
#define SA_PROFILER_BEGIN_SESSION_PATH(filepath)
#define SA_PROFILER_BEGIN_SESSION()
#define SA_PROFILER_END_SESSION()
#define SA_PROFILE_SCOPE(name)
#define SA_PROFILE_FUNCTION()
#endif

