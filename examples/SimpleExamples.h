#pragma once
#include <iostream>
#include "..\WorkerPool\WorkerPool.hpp"

void task1()
{
    std::cout << "Task 1 started on thread " << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Task 1 finished on thread " << std::this_thread::get_id() << std::endl;
}

void task2()
{
    std::cout << "Task 2 started on thread " << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Task 2 finished on thread " << std::this_thread::get_id() << std::endl;
}

void task3()
{
    std::cout << "Task 3 started on thread " << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "Task 3 finished on thread " << std::this_thread::get_id() << std::endl;
}

void AdditionTask(int a, int b)
{
    std::cout << a + b << std::endl;
}

void UsageExampleWithCalllback()
{
    ms::WorkerPool pool(2); // create a pool with 2 worker threads

    pool.AddTaskForExecution([]() {
        std::cout << "Task A started on thread " << std::this_thread::get_id() << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Task A finished on thread " << std::this_thread::get_id() << std::endl;
        });

    pool.AddTaskForExecution(task1, []() {
        std::cout << "Callback for task 1 on thread " << std::this_thread::get_id() << std::endl;
        });

    pool.AddTaskForExecution(task2, []() {
        std::cout << "Callback for task 2 on thread " << std::this_thread::get_id() << std::endl;
        });

    auto task3_fut = pool.AddTaskForExecution(task3, []() {
        std::cout << "Callback for task 3 on thread " << std::this_thread::get_id() << std::endl;
        });

    auto addition_fut = pool.AddTaskForExecution(std::bind(AdditionTask, 10, 20));

    //explicit wait on task completion:
    //if any other handling has to be done after task execution, the returned future object can be used to be waited on like following
    task3_fut.wait();
    addition_fut.wait();

}//implicitly blocks here until all the other tasks are completed