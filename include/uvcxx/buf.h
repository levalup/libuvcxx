//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_BUF_H
#define LIBUVCXX_BUF_H

#include <cstdlib>
#include <memory>

#include <uv.h>

#include "cxx/buffer_like.h"
#include "inner/base.h"

namespace uv {
    namespace inner {
        /**
         * This class is not thread safe
         */
        class buf_t : public uv_buf_t {
        public:
            using self = buf_t;
            using supper = uv_buf_t;

            using base_t = decltype(base);
            using len_t = decltype(len);

            len_t capacity = 0;

            buf_t(const buf_t &) = delete;

            buf_t &operator=(const buf_t &) = delete;

            buf_t() : supper({}) {
                this->base = nullptr;
                this->len = 0;
            }

            template<class Len, typename = typename std::enable_if_t<std::is_integral_v<Len>>>
            explicit buf_t(Len size) : self() {
                auto length = len_t(size);
                this->base = (base_t) std::malloc(size_t(length));
                this->len = length;
                this->capacity = length;
            }

            ~buf_t() {
                std::free(this->base);
            }

            explicit operator bool() const { return this->base; }

            void free() {
                if (this->base) std::free(this->base);
                this->base = nullptr;
                this->len = 0;
                this->capacity = 0;
            }

            template<class Len, typename = typename std::enable_if_t<std::is_integral_v<Len>>>
            void malloc(Len size) {
                this->realloc(size);
            }

            template<class Len, typename = typename std::enable_if_t<std::is_integral_v<Len>>>
            void realloc(Len size) {
                this->reserve(size);
                this->len = len_t(size);
            }

            template<class Len, typename = typename std::enable_if_t<std::is_integral_v<Len>>>
            void reserve(Len size) {
                auto length = len_t(size);
                if (length > this->capacity) {
                    if (this->base) {
                        this->base = (base_t) std::realloc(this->base, size_t(length));
                    } else {
                        this->base = (base_t) std::malloc(size_t(length));
                    }
                    this->capacity = length;
                }
            }
        };
    }

    /**
     * This type is a reference type, and all copied objects use the same buffer,
     *     which is not thread-safe.
     */
    class buf_t : public uvcxx::shared_raw_base_t<inner::buf_t> {
    public:
        using self = buf_t;
        using supper = uvcxx::shared_raw_base_t<inner::buf_t>;

        using raw_t = uv_buf_t;

        buf_t(std::nullptr_t) : supper(nullptr) {}

        buf_t() : supper(std::make_shared<inner::buf_t>()) {}

        template<class Size, typename = typename std::enable_if_t<std::is_integral_v<Size>>>
        explicit buf_t(Size size)
                : supper(std::make_shared<inner::buf_t>(size)) {}

        void free() { raw()->free(); }

        template<class Size, typename = typename std::enable_if_t<std::is_integral_v<Size>>>
        void malloc(Size size) { raw()->malloc(size); }

        template<class Size, typename = typename std::enable_if_t<std::is_integral_v<Size>>>
        void realloc(Size size) { raw()->realloc(size); }

        template<class Size, typename = typename std::enable_if_t<std::is_integral_v<Size>>>
        void reserve(Size size) { raw()->reserve(size); }

        char *base() { return raw()->base; }

        [[nodiscard]]
        const char *base() const { return raw()->base; }

        [[nodiscard]]
        size_t len() const { return size_t(raw()->len); }

        [[nodiscard]]
        size_t capacity() const { return size_t(raw()->capacity); }

        char *data() { return raw()->base; }

        [[nodiscard]]
        const char *data() const { return raw()->base; }

        [[nodiscard]]
        size_t size() const { return size_t(raw()->len); }

        template<class Size, typename = typename std::enable_if_t<std::is_integral_v<Size>>>
        void resize(Size size) { raw()->realloc(size); }

        void memset(int v) {
            std::memset(data(), v, size());
        }
    };
}

namespace uvcxx {
    inline buffer_like::buffer_like(uv::buf_t &buf)
            : buf(init(buf.data(), buf.size())) {}

    inline mutable_buffer_like::mutable_buffer_like(uv::buf_t &buf)
            : buf(init(buf.data(), buf.size())) {}
}

#endif //LIBUVCXX_BUF_H
