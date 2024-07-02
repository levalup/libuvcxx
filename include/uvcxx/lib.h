//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_LIB_H
#define LIBUVCXX_LIB_H

#include <atomic>
#include <cstddef>

#include <uv.h>

#include "cxx/except.h"
#include "cxx/string.h"
#include "cxx/wrapper.h"
#include "inner/base.h"
#include "utils/defer.h"

namespace uv {
    /**
     * This class is not thread-safe.
     */
    class lib_t : uvcxx::pointer_raw_base_t<uv_lib_t> {
    public:
        using self = lib_t;
        using supper = uvcxx::pointer_raw_base_t<uv_lib_t>;

        using supper::supper;

        lib_t(lib_t &&) UVCXX_NOEXCEPT = default;

        lib_t &operator=(lib_t &&) UVCXX_NOEXCEPT = default;

        lib_t(std::nullptr_t) : supper(nullptr) {}

        explicit lib_t(uvcxx::string filename) {
            UVCXX_APPLY_STRICT(uv_dlopen(filename, *this), "can not open `", filename, "`");
        }

        ~lib_t() override {
            if (*this) {
                uv_dlclose(*this);
            }
        }

        int open(uvcxx::string filename) {
            auto lib = new uv_lib_t;
            uvcxx::defer_delete<uv_lib_t> delete_lib(lib);

            UVCXX_APPLY(uv_dlopen(filename, lib), status, "can not open `", filename, "`");

            delete_lib.release();
            (void) reset_raw(lib, [](uv_lib_t *pre) {
                uv_dlclose(pre);
                delete pre;
            });
            return 0;
        }

        void close() {
            (void) reset_raw(nullptr, [](uv_lib_t *pre) {
                uv_dlclose(pre);
                delete pre;
            });
        }

        UVCXX_NODISCARD
        const char *error() const {
            if (!*this) UVCXX_THROW_OR_RETURN(UV_EPERM, "", "cannot operate on a closed lib");

            return uv_dlerror(*this);
        }

        int sym(uvcxx::string name, void **ptr) const {
            if (!*this) UVCXX_THROW_OR_RETURN(UV_EPERM, UV_EPERM, "cannot operate on a closed lib");

            UVCXX_PROXY(uv_dlsym(*this, name, ptr), "cannot find symbol `", name, "`");
        }

        template<typename FUNC, typename std::enable_if<std::is_function<FUNC>::value, int>::type = 0>
        UVCXX_NODISCARD
        FUNC *sym(uvcxx::string name) const {
            FUNC *func;
            auto err = sym(name, &func);
            if (err < 0) return nullptr;
            return func;
        }
    };
}

#endif //LIBUVCXX_LIB_H
