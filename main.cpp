
#include "ThreadPool.h"

#include <iostream>
#include <thread>
#include <cstdlib>
#include <cassert>
#include <cstdint>

void testThreadPool1();
void testThreadPool2();

template<typename T>
int64_t square(T f, int64_t i) {
    return f(i);
}

void testThreadPool1() {
    unsigned int n = std::thread::hardware_concurrency();
    std::cout << n << " concurrent threads are supported.\n";

    ThreadPool pool(n, 4);
    std::vector<std::future<int64_t>> results;

    for (int i = 0; i < 8; ++i) {
        //*
        auto square_task = [i]() -> int64_t {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            return i * i;
        };
        auto future = pool.enqueue(square_task);

        results.emplace_back(std::move(future));
        //*/
    }

    for (auto& result : results)
        std::cout << result.get() << ' ';
    std::cout << std::endl;
}

void runTests() {
    testThreadPool1();
}

int main(void) {
    runTests();

    std::cout << "Hello World!" << std::endl;

    return 0;
}
