#pragma once
#include <thread>
#include <vector>
#include <mutex>
#include <queue>
#include <algorithm>
#include <functional>
#include <semaphore>

using Task = std::function<void()>;

class WorkerPool
{
public:
	#pragma region Special member functions
	WorkerPool(unsigned int capacity = std::thread::hardware_concurrency()) : capacity(capacity), is_ready(0), cancel_flag(false), available_workers(0)
	{
		pull_task_signal = std::make_unique<std::counting_semaphore<100>>(0);
		_threads.reserve(this->capacity);
		for (unsigned int i = 0; i < this->capacity; i++)
		{
			_threads.emplace_back(&WorkerPool::routine, this);
		}
	}
	WorkerPool(const WorkerPool&) = delete; //cant allow copy as std::thread doesn't allow copy 
	WorkerPool(WorkerPool&&) = default;
	WorkerPool& operator = (const WorkerPool&) = delete;
	WorkerPool& operator = (WorkerPool&&) = default;
	~WorkerPool()
	{
		cancel_flag = true;
		pull_task_signal->release(task_queue.size());

		//This is necessary, otherwise the abort is called. you can see in the std::thread's dtor
		for (auto& t : _threads)
		{
			t.detach();
		}
	}
	#pragma endregion

	[[nodiscard]] bool IsWorkersAvailable() { return is_ready && (available_workers > 0); };

	/*
	* Adds tasks to the internal queue. If workers are available immediately the task will be executed
	* If ready workers are not available, the tasks will be executed when anyone worker thread is ready.
	* 
	* Params:
	* task_to_run - This is the task to be executed. Any callable of signature void() will be accepted
	* callback_when_complete - Calls this once the task is completed.
	* */
	void AddTaskForExecution(Task&& task_to_run, Task&& callback_when_complete)
	{
		std::unique_lock lk(tq_mx);
		task_queue.emplace(std::make_pair(task_to_run, callback_when_complete));
		lk.unlock();
		pull_task_signal->release();
	}

private:
	std::vector<std::thread> _threads;
	bool is_ready; //this is set thread safe using call_once
	bool cancel_flag;
	unsigned int capacity;
	constexpr static size_t max_threads = 100;

	void routine() noexcept;

	std::queue<std::pair<Task, Task>> task_queue;
	std::mutex tq_mx;
	/*
	* The use of unique_ptr here:
	* I was not able to use direct counting_semaphore as semaphore's copy ctor and copy assignment operators are deleted explicitly
	* */
	std::unique_ptr<std::counting_semaphore<max_threads>> pull_task_signal;
	/*
	* Not able to use condtion variable instead of semaphore for signalling the threads to pull tasks because of the lost wakeups
	* because sometimes we can have in this order: task addition to queue and then waiting on cv
	* */
	std::once_flag _isready_onceflag;
	std::atomic<int> available_workers;
};

void WorkerPool::routine() noexcept
{
	std::call_once(_isready_onceflag, [](bool& is_ready) { is_ready = true;}, is_ready);

	while (1)
	{
		available_workers++;
		try
		{
			pull_task_signal->acquire();
			if (cancel_flag) return;

			std::unique_lock lk(tq_mx);
			available_workers--;
			auto tasks = std::exchange(task_queue.front(), {});
			/*
			The use of std::exchange ensures the task_queue's front element is accessed atomically and
			any potential data races between threads accessing task_queue are avoided
			*/
			task_queue.pop();
			lk.unlock();

			//execute the work assigned
			tasks.first();

			//call the completion callback
			tasks.second();
		}
		catch (const std::exception& ex)
		{

		}
	}
}
