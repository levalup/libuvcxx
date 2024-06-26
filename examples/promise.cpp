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
    v.then([]() {}).except([](const std::exception &) {}).finally([]() {});

    {
        promise_emitter<int, int> pm(a);
        promise_cast<decltype(a), float> t(a, [](float a) {
            return std::make_tuple((int) std::floor(a), (int) std::ceil(a));
        });
        t.resolve(12.6f);
    }

    return 0;
}
