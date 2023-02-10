#pragma once

#include "taskflow\taskflow.hpp"
#include <chrono>

namespace sa {
	template<typename T>
	class ProgressView {
	private:
		std::shared_future<std::optional<T>> m_future;
		std::atomic_int m_count;
		std::atomic_int m_maxCompletionCount;

	public:
		ProgressView();

		void setFuture(const std::shared_future<std::optional<T>>& future);
		void setMaxCompletionCount(unsigned int count);


		// called by the thread/threads doing the work
		void increment();
		float getProgress() const;

		T getValue() const;
		void wait();
		void wait(const std::chrono::seconds& timeout);
		bool isDone() const;

		void reset();

		operator T();

	};

	template<typename T>
	inline ProgressView<T>::ProgressView()
		: m_count(0)
		, m_maxCompletionCount(1)
	{
	}

	template<typename T>
	inline void ProgressView<T>::setFuture(const std::shared_future<std::optional<T>>& future) {
		m_future = future;
	}

	template<typename T>
	inline void ProgressView<T>::setMaxCompletionCount(unsigned int count) {
		m_maxCompletionCount.store(count);
	}

	template<typename T>
	inline void ProgressView<T>::increment() {
		m_count.fetch_add(1, std::memory_order::memory_order_relaxed);
	}

	template<typename T>
	inline float ProgressView<T>::getProgress() const {
		return (float)m_count.load() / m_maxCompletionCount.load();
	}

	template<typename T>
	inline T ProgressView<T>::getValue() const {
		return m_future.get().value();
	}

	template<typename T>
	inline void ProgressView<T>::wait() {
		m_future.wait();
	}

	template<typename T>
	inline void ProgressView<T>::wait(const std::chrono::seconds& timeout) {
		m_future.wait_for(timeout);
	}

	template<typename T>
	inline bool ProgressView<T>::isDone() const {
		if (!m_future.valid()) 
			return true;
		return m_future._Is_ready();
	}

	template<typename T>
	inline void ProgressView<T>::reset() {
		m_count = 0;
	}

	template<typename T>
	inline ProgressView<T>::operator T() {
		return getValue();
	}

}