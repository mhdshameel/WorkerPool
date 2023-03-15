#include "SimpleExamples.h"
#include <future>


int main()
{
    UsageExampleWithCalllback();

    return 0;
}

//void UsingFutures()
//{
//    ms::WorkerPool pool;
//    std::vector<std::future<void>> futures;
//
//    // submit 10 tasks to the thread pool
//    for (int i = 0; i < 10; ++i) {
//        futures.emplace_back(pool.AddTaskForExecution([]() {
//            std::this_thread::sleep_for(std::chrono::seconds(1));
//            std::cout << "Task executed by thread " << std::this_thread::get_id() << std::endl;
//            }));
//    }
//
//    // wait for all tasks to complete
//    while (std::any_of(futures.begin(), futures.end(), [](auto& f) { return f.valid(); })) {
//        std::this_thread::sleep_for(std::chrono::milliseconds(100));
//    }
//
//    std::cout << "All tasks completed" << std::endl;
//}