#pragma once
#include "Benchmark.h"
#include <iostream>
#include <vector>
#include "..\WorkerPool\WorkerPool.hpp"
#include <numeric>
#include <functional>

long long ArraySum(const std::vector<int>& nums, unsigned int N)
{
	PROFILE_FUNCTION();
	long long sum = std::accumulate(begin(nums), end(nums), 0ll);
	return sum;
}

long long ArraySumParallelThreadPool(const std::vector<int>& nums, unsigned int N)
{
	PROFILE_FUNCTION();

	//Create the workerpool with default number of worker threads (hardware concurrency) 12 in my device
	auto threadpool = ms::WorkerPool();

	while (!threadpool.AreAllWorkersAvailable()) std::this_thread::sleep_for(std::chrono::milliseconds(1));

	auto available_hw_concurrency = std::thread::hardware_concurrency();

	unsigned int chunk_size_calculated = N / (available_hw_concurrency - 1);
	unsigned int total_processed_size = 0;

	auto process_chunk = [](std::vector<int>::const_iterator start, std::vector<int>::const_iterator end, std::shared_ptr<long long> chunk_sum_res_ptr) noexcept
	{
		//auto &[start, end, chunk_sum_res_ptr, _sem] = args;
		try {
			*chunk_sum_res_ptr = std::accumulate(start, end, 0ll);
		}
		catch (const std::exception& ex)
		{

		}
	};

	//auto sum = std::make_shared<long long>(0ll);
	auto sum = 0ll;
	std::vector<std::shared_ptr<long long>> results;
	results.reserve(20); //just for performance benefits, since shared_ptrs are used we need not worry about vector reallocation and reference invalidations

	std::counting_semaphore<25> CalculationCompletionSignal(0);

	{
		PROFILE_SCOPE("ArraySumParallelThreadPool only calculation");
		int i = 0;
		while (total_processed_size < N)
		{
			auto original_chunk_size = std::min(chunk_size_calculated, N - total_processed_size);
			auto start = nums.begin() + total_processed_size;
			auto end = start + original_chunk_size;
			total_processed_size += original_chunk_size;

			results.emplace_back(std::make_shared<long long>(0ll));
			
			threadpool.AddTaskForExecution(std::bind(process_chunk, start, end, results[i++]), [&CalculationCompletionSignal]() { CalculationCompletionSignal.release(); });
			/*
			* Bind can be used as above or a lambda capturing all the required objects can be used as well, for example 
			* [&process_chunk, start, end, &results, i]() { process_chunk(start, end, results[i]); }
			* or any callable which adheres to the std::function<void()>
			* */
		}

		//wait for all the tasks to complete
		std::for_each(results.begin(), results.end(), [&CalculationCompletionSignal](auto& val) { CalculationCompletionSignal.acquire(); });

		//calculate the individual results
		std::for_each(results.begin(), results.end(), [&sum](auto& val) { sum += *val; });
	}
	return sum;
}

void ArraySumParallelMainRoutine()
{
	START_CONSOLE_SESSION("Array Parallel vs sequential");

	constexpr unsigned int N = 500'000'000;
	std::cout << "N is " << N << std::endl;
	std::vector<int> nums(N);
	std::generate(nums.begin(), nums.end(), [n = 0]() mutable { return ++n; });

	auto sum_pl = ArraySumParallelThreadPool(nums, N);
	auto sum_seq = ArraySum(nums, N);
	if (sum_pl == sum_seq)
	{
		std::cout << "Calculation verified" << std::endl;
	}

	END_SESSION();
}

//DEBUG benchmark readings
//| Array Size | Sequential calculation time(ms) | `ms::WorkerPool` time(ms) parallel calculation | `ms::WorkerPool` time(ms) calc + threads(12) spawned
//|:-----------:|:-------------------------:|:--------------------------:|
//|     250'000 | 41 | 8 | 64 |
//|     250'000 | 45 | 9 | 35 |
//|     500'000 | 44 | 10 | 61 |
//|     500'000 |  40 | 8 | 51 |
//|     1'000'000 | 103 | 21 | 67 |
//|     1'000'000 | 131.6 | 25.9 | 68.2 |
//|     10'000'000 | 949 | 209 | 272 |
//|     10'000'000 | 844 | 230 | 301 |

//RELEASE benchmark readings
//| Array Size | Sequential calculation time(ms) | `ms::WorkerPool` time parallel calculation | `ms::WorkerPool` time calc + threads(12) initiation
//|:-----------:|:-------------------------:|:--------------------------:|
//|     1'000'000 | 1036us |  345us | 9563us |
//|     1'000'000 | 483us | 409us | 18469us |
//|     10'000'000 | 3.2ms | 3.6ms | 18.8ms |
//|     10'000'000 | 7ms | 2.4ms | 25.6ms |
//|     100'000'000 | 67.7ms | 30.3ms | 41ms |
//|     100'000'000 | 40ms | 28ms | 48.7ms |
//|     1'000'000'000 | 40ms | 28ms | 48.7ms |
//|     1'000'000'000 | 40ms | 28ms | 48.7ms |