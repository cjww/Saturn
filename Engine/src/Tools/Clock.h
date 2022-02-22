#pragma once

namespace sa {
	class Clock {
	private:
		std::chrono::high_resolution_clock::time_point m_start;

	public:
		Clock();

		template<typename T = float, typename Ratio = std::ratio<1i64>>
		T restart();

		template<typename T = float, typename Ratio = std::ratio<1i64>>
		T getElapsedTime() const;
	};

	template<typename T, typename Ratio>
	inline T Clock::restart() {
		auto now = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::duration<T, Ratio>>(now - m_start);
		m_start = now;
		return duration.count();
	}

	template<typename T, typename Ratio>
	inline T Clock::getElapsedTime() const {
		auto now = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::duration<T, Ratio>>(now - m_start);;
		return duration.count();
	}
}