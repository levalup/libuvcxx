//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_RANDOM_H
#define LIBUVCXX_RANDOM_H

#include "loop.h"
#include "req.h"

#if !UVCXX_SATISFY_VERSION(1, 33, 0)

#include <random>

#endif

namespace uv {
#if UVCXX_SATISFY_VERSION(1, 33, 0)

    class random_t : public inherit_req_t<uv_random_t, req_t> {
    public:
        using self = random_t;
        using supper = inherit_req_t<uv_random_t, req_t>;

    public:
        class data_t : public req_callback_t<raw_t, int, void *, size_t> {
        public:
            using self = data_t;
            using supper = req_callback_t<raw_t, int, void *, size_t>;

            uvcxx::promise_cast<uvcxx::promise<void *, size_t>, raw_t *, int, void *, size_t> promise;

            explicit data_t(const random_t &req)
                    : supper(req), promise([](raw_t *, int, void *buf, size_t len) {
                return std::make_tuple(buf, len);
            }) {}

            uvcxx::promise_proxy<raw_t *, int, void *, size_t> &proxy() UVCXX_NOEXCEPT override { return promise; }

            void finalize(raw_t *, int, void *, size_t) UVCXX_NOEXCEPT override {};

            int check(raw_t *, int status, void *, size_t) UVCXX_NOEXCEPT override { return status; }
        };
    };

    inline int random(std::nullptr_t, std::nullptr_t,
                      void *buf, size_t buflen, unsigned int flags,
                      std::nullptr_t) {
        return uv_random(nullptr, nullptr, buf, buflen, flags, nullptr);
    }

    UVCXX_NODISCARD
    inline uvcxx::promise<void *, size_t> random(
            const loop_t &loop, const random_t &req,
            void *buf, size_t buflen, unsigned int flags = 0) {
        auto *data = new random_t::data_t(req);
        uvcxx::defer_delete<random_t::data_t> delete_data(data);

        auto err = uv_random(loop, req, buf, buflen, flags, random_t::data_t::raw_callback);
        if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);

        delete_data.release();
        ((uv_random_t *) req)->data = data;
        return data->promise.promise();
    }

    UVCXX_NODISCARD
    inline uvcxx::promise<void *, size_t> random(
            void *buf, size_t buflen, unsigned int flags = 0) {
        return random(default_loop(), {}, buf, buflen, flags);
    }

    UVCXX_NODISCARD
    inline uvcxx::promise<void *, size_t> random(
            const loop_t &loop,
            void *buf, size_t buflen, unsigned int flags = 0) {
        return random(loop, {}, buf, buflen, flags);
    }

    UVCXX_NODISCARD
    inline uvcxx::promise<void *, size_t> random(
            const random_t &req,
            void *buf, size_t buflen, unsigned int flags = 0) {
        return random(default_loop(), req, buf, buflen, flags);
    }

#else

    inline int random(std::nullptr_t, std::nullptr_t,
                      void *buf, size_t buflen, unsigned int,
                      std::nullptr_t) {
        if (!buf || !buflen) return 0;
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<uint64_t> dist;

        auto body_size = buflen / 8;
        auto tail_size = buflen - body_size * 8;

        auto body_data = static_cast<uint64_t *>(buf);
        for (size_t i = 0; i < body_size; ++i) {
            *body_data++ = dist(rng);
        }
        if (tail_size) {
            auto tail = dist(rng);
            auto tail_data = reinterpret_cast<uint8_t *>(body_data);
            auto rand_data = reinterpret_cast<uint8_t *>(&tail);
            for (size_t i = 0; i < tail_size; ++i) {
                tail_data[i] = rand_data[i];
            }
        }

        return 0;
    }

#endif
}

#endif //LIBUVCXX_RANDOM_H
