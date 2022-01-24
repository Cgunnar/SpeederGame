#include "pch.hpp"
#include "WorkerThreads.h"



std::mutex WorkerThreads::s_workQueueMutex;
std::mutex WorkerThreads::s_highPriorityWorkQueueMutex;
std::atomic<bool> WorkerThreads::s_keepWorking = true;
std::queue<std::packaged_task<void()>> WorkerThreads::s_workQueue;
std::queue<std::packaged_task<void()>> WorkerThreads::s_highPriorityWorkQueue;
std::vector<std::thread> WorkerThreads::s_threads;
void WorkerThreads::Init(int numThreads)
{
	numThreads = std::max(std::min((int)std::thread::hardware_concurrency() - 1, numThreads), 1);
	for (int i = 0; i < numThreads; i++)
	{
		s_threads.emplace_back(Work, i);
	}

	std::cout << "Created: " << numThreads << " workthreads." << std::endl;
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
		if (s_highPriorityWorkQueueMutex.try_lock())
		{
			size_t jobs = s_highPriorityWorkQueue.size();
			if (jobs > 0)
			{
				auto task = std::move(s_highPriorityWorkQueue.front());
				s_highPriorityWorkQueue.pop();
				s_highPriorityWorkQueueMutex.unlock();
				task();

				if (jobs < s_threads.size() / 2)
					continue;
			}
			else
			{
				s_highPriorityWorkQueueMutex.unlock();
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
				std::this_thread::yield();
			}
		}
	}
}