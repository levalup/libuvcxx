//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include <cstdlib>

#include "uvcxx.h"

int main() {
    uv::mutex_t mutex = nullptr;
    uv::mutex_t other;
    mutex = std::move(other);
    int count = 0;

    uv::thread_t t1([&]() {
        for (int i = 0; i < 10; ++i) {
            mutex.lock();
            ++count;
            mutex.unlock();
            uv::sleep(std::rand() % 10);
        }
    });

    uv::thread_t t2([&]() {
        for (int i = 0; i < 10; ++i) {
            mutex.lock();
            ++count;
            mutex.unlock();
            uv::sleep(std::rand() % 10);
        }
    });

    t1.join();
    t2.join();

    uvcxx_assert(count == 20);

    return 0;
}
