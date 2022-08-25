#pragma once

#include "taskflow\taskflow.hpp"
#include <chrono>

namespace sa {
	template<typename T>
	class ProgressView {
	private:
		std::optional<T>& m_value;
		std::atomic<float> m_progress;
		std::atomic_int m_count;
		int m_finishCount;

	public:
		ProgressView(std::optional<T>& value, int finishCount);

		// called by the thread/threads doing the work
		void increment();


		float getProgress() const;

		T get();

		bool done() const;

	};

	template<typename T>
	inline ProgressView<T>::ProgressView(std::optional<T>& value, int finishCount)
		: m_finishCount(finishCount)
		, m_value(value)
		, m_count(0)
		, m_progress(0.0f)
	{
		
	}

	template<typename T>
	inline void ProgressView<T>::increment() {
		m_count++;
		float p = (float)m_count.load() / m_finishCount;
		m_progress.store(p);
	}

	template<typename T>
	inline float ProgressView<T>::getProgress() const {
		return m_progress;
	}

	template<typename T>
	inline T ProgressView<T>::get() {
		return m_value.value();
	}
	template <typename T>
	inline bool ProgressView<T>::done() const {
		return m_count >= m_finishCount;
	}

}