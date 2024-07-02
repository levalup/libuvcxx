//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include <iostream>

#include "uvcxx/async.h"

int main() {
    uv::loop_t loop;

    uv::async_t async;
    async.init(loop).call([]() {
        std::cout << "async call" << std::endl;

        throw uvcxx::close_handle();    //< close handle after once call
    });

    async.send();
    std::cout << "async send" << std::endl;

    return loop.run();
}
