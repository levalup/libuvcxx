//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_MEMORY_H
#define LIBUVCXX_MEMORY_H

#include <memory>
#include "defer.h"

namespace uvcxx {
    template<typename T>
    inline std::shared_ptr<T> make_shared(int (*init)(T *), int (*close)(T *)) {
        // The return values of malloc and init are not checked because they typically do not fail.
        auto raw = (T *) std::malloc(sizeof(T));
        defer free_raw(std::free, raw);

        if (init) init(raw);
        auto ptr =
                close
                ? std::shared_ptr<T>(raw, [close](T *p) {
                    close(p);
                    std::free(p);
                })
                : std::shared_ptr<T>(raw, [](T *p) {
                    std::free(p);
                });

        free_raw.release();
        return ptr;
    }

    template<typename T>
    inline std::shared_ptr<T> make_borrowed(T *raw) {
        return std::shared_ptr<T>(raw, [](T *) {});
    }
}

#endif //LIBUVCXX_MEMORY_H
