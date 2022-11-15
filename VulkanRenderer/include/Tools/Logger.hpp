#pragma once
#include <string>
#include <iostream>

#ifdef _WIN32
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#define DEBUG_LOG_OSTREAM std::cout

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

public:
	
	static void SetColor(FGColor foreground, BGColor background = (BGColor)0);

	template<typename ...Args>
	static void Print(Args&&... args);

	template<typename ...Args>
	static void PrintInColor(FGColor foreground, Args&&... args);
	

};

#ifdef DEBUG_LOG
#define SA_DEBUG_LOG_INFO(...) \
	Logger::SetColor(FGColor::BrightGreen); \
	DEBUG_LOG_OSTREAM << "[INFO: " << __FILENAME__ << ":" << __LINE__ << "] "; \
	Logger::SetColor(FGColor::BrightWhite); \
	Logger::Print(__VA_ARGS__); DEBUG_LOG_OSTREAM

#define SA_DEBUG_LOG_WARNING(...) \
	Logger::SetColor(FGColor::BrightYellow); \
	DEBUG_LOG_OSTREAM << "[WARNING: " << __FILENAME__ << ":" << __LINE__ << "] "; \
	Logger::SetColor(FGColor::BrightWhite); \
	Logger::Print(__VA_ARGS__); DEBUG_LOG_OSTREAM

#define SA_DEBUG_LOG_ERROR(...) \
	Logger::SetColor(FGColor::BrightRed); \
	DEBUG_LOG_OSTREAM << "[ERROR: " << __FILENAME__ << ":" << __LINE__ << "] "; \
	Logger::SetColor(FGColor::BrightWhite); \
	Logger::Print(__VA_ARGS__); DEBUG_LOG_OSTREAM

#else
#define SA_DEBUG_LOG_INFO(...)
#define SA_DEBUG_LOG_WARNING(...)
#define SA_DEBUG_LOG_ERROR(...)
#endif //  DEBUG_LOG

template<typename Arg>
inline void Logger::PrintOneArgument(Arg arg) {
	DEBUG_LOG_OSTREAM << arg;
}

template<typename ...Args>
inline void Logger::Print(Args&& ...args) {
	int dummy[] = { 0, ( (void)PrintOneArgument(std::forward<Args>(args)), 0) ... };
	DEBUG_LOG_OSTREAM << std::endl;
}

template<typename ...Args>
inline void Logger::PrintInColor(FGColor color, Args&&... args) {
	SetColor(color);
	Print(args...);
	SetColor((FGColor)0);
}
