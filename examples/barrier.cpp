//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include <iostream>
#include <chrono>

#include "uvcxx/barrier.h"
#include "uvcxx/thread.h"
#include "uvcxx/utilities.h"

int main() {
    uv::barrier_t barrier(2);
    auto beg = std::chrono::steady_clock::now();

    uv::thread_t thread1([&barrier, beg]() {
        std::cout << uvcxx::catstr("thread-1 wait ", 0, "ms\n") << std::flush;
        barrier.wait();
        auto end = std::chrono::steady_clock::now();
        auto spent = std::chrono::duration_cast<std::chrono::milliseconds>(end - beg).count();
        std::cout << uvcxx::catstr("thread-1 exit after ", spent, "ms\n") << std::flush;
    });

    uv::thread_t thread2([&barrier, beg]() {
        std::cout << uvcxx::catstr("thread-2 wait ", 100, "ms\n") << std::flush;
        uv::sleep(100);
        barrier.wait();
        auto end = std::chrono::steady_clock::now();
        auto spent = std::chrono::duration_cast<std::chrono::milliseconds>(end - beg).count();
        std::cout << uvcxx::catstr("thread-2 exit after ", spent, "ms\n") << std::flush;
    });

    thread1.join();
    thread2.join();

    return 0;
}
