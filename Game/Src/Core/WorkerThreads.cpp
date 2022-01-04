#include "pch.hpp"
#include "WorkerThreads.h"



std::mutex WorkerThreads::s_workQueueMutex;
std::atomic<bool> WorkerThreads::s_keepWorking = true;
std::queue<std::packaged_task<void()>> WorkerThreads::s_workQueue;
std::vector<std::thread> WorkerThreads::s_threads;
void WorkerThreads::Init(int numThreads)
{
	for (int i = 0; i < numThreads; i++)
	{
		s_threads.emplace_back(Work);
	}
}

void WorkerThreads::Destroy()
{
	s_keepWorking = false;
	for (auto& t : s_threads)
	{
		t.join();
	}
}

void WorkerThreads::Work()
{
	while (s_keepWorking)
	{
		s_workQueueMutex.lock();
		if (!s_workQueue.empty())
		{
			auto task = std::move(s_workQueue.front());
			s_workQueue.pop();
			s_workQueueMutex.unlock();
			task();
		}
		else
		{
			s_workQueueMutex.unlock();
			std::this_thread::yield();
		}
	}
}