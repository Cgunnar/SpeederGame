#pragma once

class WorkerThreads
{
public:
	static void Init(int numThreads = std::thread::hardware_concurrency() / 2);
	static void Destroy();

	template<typename F, typename ... Args>
	static void AddTask(F&& func, Args&&... args);

private:
	static std::queue<std::packaged_task<void()>> s_workQueue;
	static void Work();
	static std::vector<std::thread> s_threads;
	static std::mutex s_workQueueMutex;
	static std::atomic<bool> s_keepWorking;
};



template<typename F, typename ...Args>
inline void WorkerThreads::AddTask(F&& func, Args&&... args)
{
	if (s_keepWorking)
	{
		s_workQueueMutex.lock();
		s_workQueue.emplace(
			[func = std::forward<F>(func), args = std::make_tuple(std::forward<Args>(args)...)]()  mutable
		{
			std::apply(func, std::move(args));
		});
		s_workQueueMutex.unlock();
	}
}
