#pragma once

#include <queue>
#include <vector>
#include <functional>
#include <thread>
#include <mutex>

class ThreadPool
{
	std::queue<std::function<void()>> tasks;

	std::vector<std::thread> workers;
	size_t workersBusy;

	std::mutex mutex;
	std::condition_variable condition;

	bool bQuit;

public:

	ThreadPool(size_t numWorkers) : workersBusy(numWorkers), bQuit(false)
	{
		for (size_t i = 0; i < numWorkers; i++) {
			workers.emplace_back(
				[this] {
					while (true) {
						std::function<void()> task;
						{
							std::unique_lock<std::mutex> lock(mutex);
							workersBusy--;
							condition.wait(lock, [this] { return bQuit || !tasks.empty(); });
							if (bQuit && tasks.empty()) return;
							workersBusy++;
							task = std::move(tasks.front());
							tasks.pop();
						}
						task();
					}
				}
			);
		}
	}

	~ThreadPool()
	{
		{
			std::unique_lock<std::mutex> lock(mutex);
			bQuit = true;
		}
		condition.notify_all();
		for (auto& worker : workers) worker.join();
	}

	void enqueue(std::function<void()> task)
	{
		{
			std::unique_lock<std::mutex> lock(mutex);
			tasks.emplace(task);
		}
		condition.notify_one();
	}

	void waitForTasks()
	{
		std::unique_lock<std::mutex> lock(mutex);
		condition.wait(lock, [this] { return !workersBusy && tasks.empty(); });
	}
};
