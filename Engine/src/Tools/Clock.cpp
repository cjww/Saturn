#include "pch.h"
#include "Clock.h"

sa::Clock::Clock() {
	m_start = std::chrono::high_resolution_clock::now();
}
