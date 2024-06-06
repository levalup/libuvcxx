//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include <uvcxx.h>

int main() {
    uv::timer_t().start(1000, 1000).call([](){
        std::cout << "Hello~~~" << std::endl;
        throw uvcxx::close_handle();
    });
    return uv::default_loop().run();
}