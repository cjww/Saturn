#pragma once
#include <string>
#include <iostream>

#ifdef _WIN32
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

enum class FGColor {
	Black           = 30,
	Red             = 31,
	Green           = 32,
	Yellow          = 33,
	Blue            = 34,
	Magenta         = 35,
	Cyan            = 36,
	White           = 37,
	BrightBlack		= 90, 
	BrightRed		= 91, 
	BrightGreen		= 92, 
	BrightYellow	= 93, 
	BrightBlue		= 94, 
	BrightMagenta	= 95, 
	BrightCyan		= 96, 
	BrightWhite		= 97 
};

enum class BGColor {
	Black			= 40,
	Red				= 41,
	Green			= 42,
	Yellow			= 43,
	Blue			= 44,
	Magenta			= 45,
	Cyan			= 46,
	White			= 47,
	BrightBlack		= 100,
	BrightRed		= 101,
	BrightGreen		= 102,
	BrightYellow	= 103,
	BrightBlue		= 104,
	BrightMagenta	= 105,
	BrightCyan		= 106,
	BrightWhite		= 107
};

class Logger {
private:

	template<typename Arg>
	static void PrintOneArgument(Arg arg);

	inline static std::ostream* s_outStream = &std::cout;
	inline static std::mutex s_loggerMutex;

public:
	
	static void SetColor(FGColor foreground, BGColor background = (BGColor)0);
	static void SetOutStream(std::ostream& outStream);

	template<typename ...Args>
	static void Print(Args&&... args);

	template<typename ...Args>
	static void PrintInColor(FGColor foreground, Args&&... args);
	
	template<typename ...Args>
	static void PrintInfo(const char* filename, int line, Args&&... args);

	template<typename ...Args>
	static void PrintWarning(const char* filename, int line, Args&&... args);

	template<typename ...Args>
	static void PrintError(const char* filename, int line, Args&&... args);


};

#if SA_DEBUG_LOG_ENABLE
#define SA_DEBUG_LOG_INFO(...) Logger::PrintInfo(__FILENAME__, __LINE__, __VA_ARGS__)
#define SA_DEBUG_LOG_WARNING(...) Logger::PrintWarning(__FILENAME__, __LINE__, __VA_ARGS__)
#define SA_DEBUG_LOG_ERROR(...) Logger::PrintError(__FILENAME__, __LINE__, __VA_ARGS__)
#else
#define SA_DEBUG_LOG_INFO(...)
#define SA_DEBUG_LOG_WARNING(...)
#define SA_DEBUG_LOG_ERROR(...)
#endif //  DEBUG_LOG

template<typename Arg>
inline void Logger::PrintOneArgument(Arg arg) {
	(*s_outStream) << arg;
}

template<typename ...Args>
inline void Logger::Print(Args&& ...args) {
	int dummy[] = { 0, ( (void)PrintOneArgument(std::forward<Args>(args)), 0) ... };
	(*s_outStream) << std::endl;
}

template<typename ...Args>
inline void Logger::PrintInColor(FGColor color, Args&&... args) {
	SetColor(color);
	Print(args...);
	SetColor((FGColor)0);
}


template<typename ...Args>
inline static void Logger::PrintInfo(const char* filename, int line, Args&&... args) {
	const std::lock_guard<std::mutex> lock(s_loggerMutex);
	SetColor(FGColor::BrightGreen);
	(*s_outStream) << "[INFO: " << filename << ":" << line << "] ";
	SetColor(FGColor::BrightWhite);
	Print(args...);
}

template<typename ...Args>
inline static void Logger::PrintWarning(const char* filename, int line, Args&&... args) {
	const std::lock_guard<std::mutex> lock(s_loggerMutex);
	SetColor(FGColor::BrightYellow);
	(*s_outStream) << "[WARNING: " << filename << ":" << line << "] ";
	SetColor(FGColor::BrightWhite);
	Print(args...);
}

template<typename ...Args>
inline static void Logger::PrintError(const char* filename, int line, Args&&... args) {
	const std::lock_guard<std::mutex> lock(s_loggerMutex);
	Logger::SetColor(FGColor::BrightRed);
	(*s_outStream) << "[ERROR: " << filename << ":" << line << "] ";
	Logger::SetColor(FGColor::BrightWhite);
	Logger::Print(args...);
}