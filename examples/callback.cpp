//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include <cmath>

#include "uvcxx/utils/callback.h"

int main() {
    using namespace uvcxx;

    callback<int, int> a;
    a.call([](int a, int b) {
        std::cout << a << " " << b << std::endl;
    });

    callback_cast<decltype(a), float> cast(
            a, [](float a) {
                return std::make_tuple((float) std::floor(a), (float) std::ceil(a));
            });
    cast(12.4f);

    return 0;
}
