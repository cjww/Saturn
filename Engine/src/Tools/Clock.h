#pragma once

namespace sa {
	class Clock {
	private:
		std::chrono::high_resolution_clock::time_point m_start;

	public:
		Clock();

		template<typename T, typename P = std::ratio<1i64>>
		T restart();

		template<typename T, typename P>
		T getElapsedTime() const;
	};

	template<typename T, typename P>
	inline T Clock::restart() {
		auto now = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::duration<T, P>>(now - m_start);
		m_start = now;
		return duration.count();
	}

	template<typename T, typename P>
	inline T Clock::getElapsedTime() const
	{
		return T();
	}
}