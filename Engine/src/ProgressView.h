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

		std::vector<const ProgressView<T>*> m_dependencies;

		unsigned int getMaxCompletionCount() const;
		unsigned int getAllMaxCompletionCount() const;
		unsigned int getAllCompletionCount() const;
	public:
		ProgressView();

		void setFuture(const std::shared_future<std::optional<T>>& future);
		void setMaxCompletionCount(unsigned int count);


		// called by the thread/threads doing the work
		void increment();
		float getCompletion() const;
		float getAllCompletion() const;

		T getValue() const;
		void wait();
		void wait(const std::chrono::seconds& timeout);
		void waitAll();
		void waitAll(const std::chrono::seconds& timeout);


		bool isDone() const;
		bool isAllDone() const;

		void reset();

		void addDependency(const ProgressView<T>* pDependency);

		operator T();

	};

	template<typename T>
	inline unsigned int ProgressView<T>::getMaxCompletionCount() const {
		return m_maxCompletionCount;
	}

	template<typename T>
	inline unsigned int ProgressView<T>::getAllMaxCompletionCount() const {
		unsigned int max = m_maxCompletionCount;
		for (const auto& dep : m_dependencies) {
			max += dep->getAllMaxCompletionCount();
		}
		return max;
	}

	template<typename T>
	inline unsigned int ProgressView<T>::getAllCompletionCount() const {
		unsigned int count = m_count;
		for (const auto& dep : m_dependencies) {
			count += dep->getAllCompletionCount();
		}
		return count;
	}


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
	inline float ProgressView<T>::getCompletion() const {
		return (float)m_count.load() / m_maxCompletionCount.load();
	}

	template<typename T>
	inline float ProgressView<T>::getAllCompletion() const {
		unsigned int max = getAllMaxCompletionCount();
		unsigned int count = getAllCompletionCount();
		return (float) count / max;
	}

	template<typename T>
	inline T ProgressView<T>::getValue() const {
		return m_future.get().value();
	}

	template<typename T>
	inline void ProgressView<T>::wait() {
		if(m_future.valid())
			m_future.wait();
	}

	template<typename T>
	inline void ProgressView<T>::wait(const std::chrono::seconds& timeout) {
		m_future.wait_for(timeout);
	}

	template<typename T>
	inline void ProgressView<T>::waitAll() {
		wait();
		for (auto& dep : m_dependencies) {
			dep->wait();
		}
	}

	template<typename T>
	inline void ProgressView<T>::waitAll(const std::chrono::seconds& timeout) {
		wait(timeout);
		for (auto& dep : m_dependencies) {
			dep->wait(timeout);
		}
	}

	template<typename T>
	inline bool ProgressView<T>::isDone() const {
		if (!m_future.valid()) 
			return true;
		return m_future._Is_ready();
	}

	template<typename T>
	inline bool ProgressView<T>::isAllDone() const {
		if (!isDone())
			return false;

		for (auto& dep : m_dependencies) {
			if (!dep->isAllDone())
				return false;
		}

		return true;
	}

	template<typename T>
	inline void ProgressView<T>::reset() {
		m_count = 0;
		m_dependencies.clear();
	}

	template<typename T>
	inline void ProgressView<T>::addDependency(const ProgressView<T>* pDependency) {
		m_dependencies.push_back(pDependency);
	}

	template<typename T>
	inline ProgressView<T>::operator T() {
		return getValue();
	}

}