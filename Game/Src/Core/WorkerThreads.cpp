#include "pch.hpp"
#include "WorkerThreads.h"



std::mutex WorkerThreads::s_workQueueMutex;
std::mutex WorkerThreads::s_fastWorkQueueMutex;
std::atomic<bool> WorkerThreads::s_keepWorking = true;
std::queue<std::packaged_task<void()>> WorkerThreads::s_workQueue;
std::queue<std::packaged_task<void()>> WorkerThreads::s_fastWorkQueue;
std::vector<std::thread> WorkerThreads::s_threads;
void WorkerThreads::Init(int numThreads)
{
	for (int i = 0; i < numThreads; i++)
	{
		s_threads.emplace_back(Work, i);
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

void WorkerThreads::Work(int id)
{
	while (s_keepWorking)
	{
		if (s_fastWorkQueueMutex.try_lock())
		{
			size_t jobs = s_fastWorkQueue.size();
			if (jobs > 0)
			{
				auto task = std::move(s_fastWorkQueue.front());
				s_fastWorkQueue.pop();
				s_fastWorkQueueMutex.unlock();
				task();

				if (jobs < s_threads.size() / 2)
					continue;
			}
			else
			{
				s_fastWorkQueueMutex.unlock();
			}
		}
		else if (s_workQueueMutex.try_lock())
		{
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
				/*std::this_thread::yield();*/
			}
		}
	}
}