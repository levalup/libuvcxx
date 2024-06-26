//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_BUF_H
#define LIBUVCXX_BUF_H

#include <cstdlib>
#include <memory>

#include <uv.h>

#include "cxx/buffer.h"
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

            template<class Len, typename std::enable_if<std::is_integral<Len>::value, int>::type = 0>
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

            template<class Len, typename std::enable_if<std::is_integral<Len>::value, int>::type = 0>
            void malloc(Len size) {
                this->realloc(size);
            }

            template<class Len, typename std::enable_if<std::is_integral<Len>::value, int>::type = 0>
            void realloc(Len size) {
                this->reserve(size);
                this->len = len_t(size);
            }

            template<class Len, typename std::enable_if<std::is_integral<Len>::value, int>::type = 0>
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

        template<class Size, typename std::enable_if<std::is_integral<Size>::value, int>::type = 0>
        explicit buf_t(Size size)
                : supper(std::make_shared<inner::buf_t>(size)) {}

        void free() { raw()->free(); }

        template<class Size, typename std::enable_if<std::is_integral<Size>::value, int>::type = 0>
        void malloc(Size size) { raw()->malloc(size); }

        template<class Size, typename std::enable_if<std::is_integral<Size>::value, int>::type = 0>
        void realloc(Size size) { raw()->realloc(size); }

        template<class Size, typename std::enable_if<std::is_integral<Size>::value, int>::type = 0>
        void reserve(Size size) { raw()->reserve(size); }

        char *base() { return raw()->base; }

        UVCXX_NODISCARD
        const char *base() const { return raw()->base; }

        UVCXX_NODISCARD
        size_t len() const { return size_t(raw()->len); }

        UVCXX_NODISCARD
        size_t capacity() const { return size_t(raw()->capacity); }

        char *data() { return raw()->base; }

        UVCXX_NODISCARD
        const char *data() const { return raw()->base; }

        UVCXX_NODISCARD
        size_t size() const { return size_t(raw()->len); }

        template<class Size, typename std::enable_if<std::is_integral<Size>::value, int>::type = 0>
        void resize(Size size) { raw()->realloc(size); }

        void memset(int v) {
            std::memset(data(), v, size());
        }
    };

    template<class I, typename std::enable_if<std::is_integral<I>::value, int>::type = 0>
    inline uv_buf_t buf_init(const void *base, I len) {
        // cover uv_buf_init
        uv_buf_t buf{};
        buf.base = (decltype(buf.base)) (base);
        buf.len = (decltype(buf.len)) (len);
        return buf;
    }
}

namespace uvcxx {
    inline buffer::buffer(uv::buf_t &buf)
            : buf(init(buf.data(), buf.size())) {}

    inline mutable_buffer::mutable_buffer(uv::buf_t &buf)
            : buf(init(buf.data(), buf.size())) {}
}

#endif //LIBUVCXX_BUF_H
