//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include "uvcxx/cxx/promise.h"

int main() {
    using namespace uvcxx;

    promise<int> a;
    a.then([](int i) {
        std::cout << "got " << i << std::endl;
        throw std::logic_error("i");
    });
    promise<void> v;
    v.then([]() {}).except([](const std::exception &e) {}).finally([](){});

    {
        promise_emitter pm(a);
        promise_cast<int, float> t(a, [](float a){ return std::ceil(a); });
        t.resolve(12.6);
    }

    return 0;
}