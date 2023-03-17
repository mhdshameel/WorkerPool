#include "pch.h"
#include "..\WorkerPool\WorkerPool.hpp"
#include <chrono>
#include <thread>
#include <atomic>

using namespace ms;

TEST(WorkerPoolTests, AddTaskTest)
{
    WorkerPool pool(2);
    std::atomic<int> counter(0);

    pool.AddTaskForExecution([&counter]() { counter++; });

    // Wait for task to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_EQ(counter, 1);
}

TEST(WorkerPoolTests, MultipleTasksTest)
{
    WorkerPool pool(2);
    std::atomic<int> counter(0);

    pool.AddTaskForExecution([&counter]() { counter++; });
    pool.AddTaskForExecution([&counter]() { counter++; });
    pool.AddTaskForExecution([&counter]() { counter++; }, [&counter]() { counter++; }); //tests callback also

    // Wait for tasks to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_EQ(counter, 4);
}

TEST(WorkerPoolTests, TasksWaitOnExitTest)
{
    std::atomic<int> counter(0);

    {
        WorkerPool pool(2);

        auto task = [&counter]() { counter++; };
        auto task2 = [&counter]() { counter++; };

        pool.AddTaskForExecution(task);
        pool.AddTaskForExecution(task2);
    }

    EXPECT_EQ(counter, 2);
}

TEST(WorkerPoolTests, CallbackTest)
{
    WorkerPool pool(2);
    std::atomic<int> counter(0);

    auto task = [&counter]() { counter++; };
    auto callback = [&counter]() { counter++; };

    pool.AddTaskForExecution(task, callback);

    // Wait for task and callback to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_EQ(counter, 2);
}

TEST(WorkerPoolTests, BlockingTaskTest)
{
    std::atomic<int> counter(0);
    {
        WorkerPool pool(2);

        auto blocking_task = []() { std::this_thread::sleep_for(std::chrono::milliseconds(100)); };
        auto task = [&counter]() { counter++; };

        pool.AddTaskForExecution(blocking_task);
        pool.AddTaskForExecution(task);
    }
    //should wait for task completion here

    EXPECT_EQ(counter, 1);
}

TEST(WorkerPoolTests, PoolThreadSafetyTest1)
{
    std::atomic<int> counter(0);
    constexpr int N = 100;

    {
        WorkerPool pool(N);

        auto test = [&counter, &pool, &N] {
            for (int i = 0; i < N; i++)
            {
                pool.AddTaskForExecution([&counter] {
                    counter++;
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    });
            }
        };

        for (int i = 0; i < N; i++)
        {
            pool.AddTaskForExecution(test);
        }
    }

    EXPECT_EQ(counter, N * N);
}

TEST(WorkerPoolTests, FuturesWaitTests)
{
    ms::WorkerPool pool;
    std::vector<std::future<void>> futures;
    std::atomic<int> counter(0);

    // submit 10 tasks to the thread pool
    for (int i = 0; i < 10; ++i) {
        futures.emplace_back(pool.AddTaskForExecution([&counter]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            counter++;
            }));
    }

    // wait for all tasks to complete
    //std::for_each(futures.begin(), futures.end(), [](auto& f) { f.wait();});

    while (std::any_of(futures.begin(), futures.end(), [](auto& f) { f.wait(); return true; })) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    EXPECT_EQ(counter, 10);
}


TEST(WorkerPoolTests, 1000ThreadsTest)
{
    std::atomic<int> counter(0);
    constexpr int N = 1'000;

    {
        WorkerPool pool(N);

        for (int i = 0; i < N; i++)
        {
            pool.AddTaskForExecution([&counter] {
                counter++;
                });
        }
    }

    EXPECT_EQ(counter, N);
}
