#include "pch.h"
#include "Tools/Logger.hpp"

char setColorStr[11];


void Logger::SetColor(FGColor foreground, BGColor background) {
	snprintf(setColorStr, 11, "\033[%d;%dm", (int)background, (int)foreground);
	printf(setColorStr);
}

void Logger::SetOutStream(std::ostream& outStream) {
	s_outStream = &outStream;
}
