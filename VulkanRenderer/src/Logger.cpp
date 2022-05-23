#include "pch.h"
#include "Tools/Logger.hpp"

void Logger::SetColor(FGColor foreground, BGColor background) {
	std::string str = "\033[" + std::to_string((int)background) + ";" + std::to_string((int)foreground) + "m";
	printf(str.c_str());
}
