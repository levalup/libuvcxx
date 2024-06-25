//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include <iostream>
#include <cassert>

#include "uvcxx/thread.h"
#include "uvcxx/key.h"
#include "uvcxx/utilities.h"

int main() {
    // create thread local storage key
    // key_t is not copyable
    uv::key_t tls;

    uv::thread_t t1([&]() {
        auto v = new int(1);

        tls.set(v);

        uv::sleep(1000);    // wait thread-2 set value
        std::cout << "Thread-1 got " << tls.ref<int>() << std::endl;

        assert(tls.ref<int>() == 1);

        delete v;
    });
    uv::thread_t t2([&]() {
        auto v = new int(2);

        uv::sleep(500);    // wait thread-1 set value
        tls.set(v);

        uv::sleep(1000);    // wait thread-1 get value
        std::cout << "Thread-2 got " << tls.ref<int>() << std::endl;

        assert(tls.ref<int>() == 2);

        delete v;
    });

    t1.join();
    t2.join();

    return 0;
}
