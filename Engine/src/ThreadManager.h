#pragma once

#include <thread>
#include <vector>
namespace sa {

	class ThreadPool {
	private:
		std::vector<std::thread> m_threads;
		std::vector<int> m_free;
	public:
		ThreadPool(size_t noThreads);
		void allocate(size_t noThreads);
		std::thread getNext();
	};

	class ThreadManager {
	private:
		ThreadPool m_threadPool;
	public:
		ThreadManager();

	};
}