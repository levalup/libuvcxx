//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include <cmath>

#include "uvcxx/utils/promise.h"

int main() {
    using namespace uvcxx;

    promise<int, int> a;
    a.then([](int i, int j) {
        std::cout << "got " << i << " " << j << std::endl;
        throw std::logic_error("i");
    });
    promise<> v;
    v.then([]() {}).except([](const std::exception &e) {}).finally([]() {});

    {
        promise_emitter pm(a);
        promise_cast<decltype(a), float> t(a, [](float a) {
            return std::make_tuple(std::floor(a), std::ceil(a));
        });
        t.resolve(12.6);
    }

    return 0;
}
