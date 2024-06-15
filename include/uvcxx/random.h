//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_RANDOM_H
#define LIBUVCXX_RANDOM_H

#include "loop.h"
#include "req.h"

namespace uv {
    class random_t : public inherit_req_t<uv_random_t, req_t> {
    public:
        using self = random_t;
        using supper = inherit_req_t<uv_random_t, req_t>;

        class data_t : public req_callback_t<raw_t, int, void *, size_t> {
        public:
            using self = data_t;
            using supper = req_callback_t<raw_t, int, void *, size_t>;

            uvcxx::promise_cast<uvcxx::promise<void *, size_t>, raw_t *, int, void *, size_t> promise;

            explicit data_t(const random_t &req)
                    : supper(req), promise([](raw_t *, int, void *buf, size_t len) {
                return std::make_tuple(buf, len);
            }) {
            }

            uvcxx::promise_proxy<raw_t *, int, void *, size_t> &proxy() noexcept override { return promise; }

            void finalize(raw_t *, int, void *, size_t) noexcept override {};

            int check(raw_t *, int status, void *, size_t) noexcept override { return status; }
        };
    };

    [[nodiscard]]
    inline uvcxx::promise<void *, size_t> random(
            const loop_t &loop, const random_t &req,
            void *buf, size_t buflen, unsigned int flags) {
        auto *data = new random_t::data_t(req);
        uvcxx::defer_delete delete_data(data);

        auto err = uv_random(loop, req, buf, buflen, flags, random_t::data_t::raw_callback);
        if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);

        delete_data.release();
        return data->promise.promise();
    }

    [[nodiscard]]
    inline uvcxx::promise<void *, size_t> random(
            void *buf, size_t buflen, unsigned int flags) {
        return random(default_loop(), {}, buf, buflen, flags);
    }

    [[nodiscard]]
    inline uvcxx::promise<void *, size_t> random(
            const loop_t &loop,
            void *buf, size_t buflen, unsigned int flags) {
        return random(loop, {}, buf, buflen, flags);
    }

    [[nodiscard]]
    inline uvcxx::promise<void *, size_t> random(
            const random_t &req,
            void *buf, size_t buflen, unsigned int flags) {
        return random(default_loop(), req, buf, buflen, flags);
    }
}

#endif //LIBUVCXX_RANDOM_H
