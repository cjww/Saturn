#pragma once

#ifdef _WIN32
//#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
typedef _CrtMemState MemoryState;
#else
typedef int MemoryState;
#endif // _WIN32

class MemoryChecker {
	MemoryState m_startState;
	MemoryState m_endState;
	MemoryState m_diff;
public:
	MemoryChecker();
	~MemoryChecker();
};

