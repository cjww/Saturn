#include "pch.h"
#include "MemoryChecker.h"

MemoryChecker::MemoryChecker() {
#ifdef _WIN32
	_CrtMemCheckpoint(&m_startState);
#endif // _WIN32

}

MemoryChecker::~MemoryChecker() {
#ifdef _WIN32
	_CrtMemCheckpoint(&m_endState);
	if (_CrtMemDifference(&m_diff, &m_startState, &m_endState)) {
		_CrtMemDumpStatistics(&m_diff);
		//_CrtMemDumpAllObjectsSince(&oldState);
		_CrtDumpMemoryLeaks();
	}
#endif // _WIN32
}
