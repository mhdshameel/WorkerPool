# WorkerPool
WorkerPool is a header-only C++ library for managing a pool of worker threads that execute tasks asynchronously. The library provides a simple interface for adding tasks to the pool, and automatically manages the worker threads to ensure that they are utilized efficiently.

## Features
 * Flexible task scheduling: Tasks can be added to the pool at any time, and will be executed as soon as a worker thread becomes available.
 * Automatic thread management: The library automatically creates and manages a pool of worker threads based on the `capacity` specified during creation or hardware concurrency of the system.
 * Perfomant: Avoid thread instantiation overhead for the tasks that can run asynchronously or the tasks that can be offloaded to run parallely to the available worker threads that immediately execute the task assigned.
 * Efficient thread signaling: The library uses a counting semaphore (C++20) to signal worker threads when tasks are added to the pool, ensuring that threads are only woken up when there are tasks to execute.
 * Flexible task completion: An optional callable can be provided to the library to be executed when the assigned task completes.
 * Threadsafe - The library is threadsafe itself and can be accessed concurrently from multiple threads without a worry. 
 * Clean shutdown: The library provides a clean shutdown mechanism that allows tasks to complete before terminating worker threads.
 * Header-only: The library is implemented entirely in header files, making it easy to integrate into existing projects.
 
Check the `lite` branch for only the header file without other Windows project management stuffs. I will try to keep the lite branch updated.

## Usage
Here's a simple example of how to use the WorkerPool library:

```
#include <iostream>
#include <chrono>
#include "WorkerPool.h" //Include the library header

void task1()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout << "Task 1 executed" << std::endl;
}

void task2()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

int main()
{
    // create a worker pool with default capacity (hardware concurrency)
    ms::WorkerPool pool;

    // add two tasks to the pool
    auto fut1 = pool.AddTaskForExecution(task1);
    auto fut2 = pool.AddTaskForExecution(task2, [](){std::cout << "Task 2 executed" << std::endl;}); //optionally you can add a callback to be called once the task completes

    // wait for tasks to complete
    fut1.wait();
    fut2.wait();

    return 0;
}
```

The `examples` folder contains more sophisticated examples showcasing the usage of the different features in play.

## Building
The WorkerPool library is implemented entirely in header files, so there is no need to build or link against any external libraries. Simply include the header file in your source code, and you're ready to go. C++20 and above is required (another version of the library targeting C++17 or lower in a separate branch will be ready soon).

The library has been tested with the following compilers:

* Visual C++ 2019 (Windows)
* GCC 10.3 (Ubuntu)
* Clang 12.0 (Ubuntu)

## Things to be avoided or used carefully
While using this library, it is important to note the following things to be used at the user's own discretion:

Blocking tasks: The library is designed to execute tasks asynchronously, so it is important to avoid adding tasks that block for a long time (e.g. I/O operations, sleep statements, etc.).
Infinite loops: Tasks should not contain infinite loops, as this will cause the worker threads to become stuck and prevent other tasks from executing. And it will prevent the application from properly terminating.

## License

The project is licensed under the MIT license. See [LICENSE](LICENSE) for more details.
