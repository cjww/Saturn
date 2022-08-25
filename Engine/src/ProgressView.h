#pragma once

#include "taskflow\taskflow.hpp"
#include <chrono>

namespace sa {
	template<typename T>
	class ProgressView {
	private:
		std::shared_future<std::optional<T>> m_future;
		std::shared_ptr<std::atomic_int> m_count;
		std::shared_ptr<unsigned int> m_maxCompletionCount;

	public:
		ProgressView();
		//ProgressView(ProgressView<T>&&) = default;

		void setFuture(const std::shared_future<std::optional<T>>& future);
		void setMaxCompletionCount(unsigned int count);


		// called by the thread/threads doing the work
		//void setProgress(float progress);
		void increment();
		float getProgress() const;

		T get() const;
		void wait();
		void wait(const std::chrono::seconds& timeout);
		bool isDone() const;

	};

	template<typename T>
	inline ProgressView<T>::ProgressView()
		: m_count(std::make_shared<std::atomic_int>(0))
		, m_maxCompletionCount(std::make_shared<unsigned int>(1U))
	{
	}

	template<typename T>
	inline void ProgressView<T>::setFuture(const std::shared_future<std::optional<T>>& future) {
		m_future = future;
	}

	template<typename T>
	inline void ProgressView<T>::setMaxCompletionCount(unsigned int count) {
		*m_maxCompletionCount = count;
	}
	
	template<typename T>
	inline void ProgressView<T>::increment() {
		m_count->fetch_add(1, std::memory_order::memory_order_relaxed);
	}

	template<typename T>
	inline float ProgressView<T>::getProgress() const {
		return (float)m_count->load() / *m_maxCompletionCount;
	}

	template<typename T>
	inline T ProgressView<T>::get() const {
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
		return m_future._Is_ready();
	}

}