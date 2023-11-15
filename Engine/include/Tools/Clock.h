#pragma once
#include <chrono>

namespace sa {
	class Clock {
	private:
		std::chrono::high_resolution_clock::time_point m_start;

	public:
		Clock();

		template<typename Duration = std::chrono::duration<float>>
		auto getStartTime();

		template<typename Duration = std::chrono::duration<float>>
		auto restart();

		template<typename Duration = std::chrono::duration<float>>
		auto getElapsedTime() const;
	};

	template<typename Duration>
	inline auto Clock::getStartTime() {
		return std::chrono::time_point_cast<Duration>(m_start).time_since_epoch().count();
	}

	template<typename Duration>
	inline auto Clock::restart() {
		auto now = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<Duration>(now - m_start);
		m_start = now;
		return duration.count();
	}

	template<typename Duration>
	inline auto Clock::getElapsedTime() const {
		auto now = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<Duration>(now - m_start);
		return duration.count();
	}
}