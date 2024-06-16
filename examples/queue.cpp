//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include <iostream>

#include "uvcxx/utils/queue.h"
#include "uvcxx/utils/callback.h"

int main() {
    using namespace uvcxx;

    callback<int, float> cb;
    queue<int, float> q = cb.get_queue();

    q.push(1, 0);
    q.push(1, 2);
    q.close();
    for (auto [i, j]: q) {
        std::cout << i << " " << j << std::endl;
    }

    return 0;
}
