//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include <uvcxx.h>

void detach_work() {
    // uv::timer_t()      // Create handle
    // .start(1000, 1000) // Start timer
    // .detach()          // Detached handle won't be implicitly closed
    // .call(...)         // Set callback function
    uv::timer_t().start(100, 1).detach().call([]() {
        std::cout << "Hello,";
        // Tell the associated handle to close
        throw uvcxx::close_handle();
    });

    // Classic implementation
    uv::timer_t timer;
    timer.start(200, 1).call([=]() mutable {
        std::cout << " World!" << std::endl;
        timer.close();  // Capture timer in callback function is safe ^_^.
    });
    timer.detach();
    // If detach is not called, the temporary timer will be automatically closed.
}

int main() {
    detach_work();
    return uv::default_loop().run();
}
