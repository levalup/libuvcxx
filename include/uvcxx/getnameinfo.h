//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_GETNAMEINFO_H
#define LIBUVCXX_GETNAMEINFO_H

#include "loop.h"
#include "req.h"

namespace uv {
    class getnameinfo_t : public inherit_req_t<uv_getnameinfo_t, req_t> {
    public:
        using self = getnameinfo_t;
        using supper = inherit_req_t<uv_getnameinfo_t, req_t>;

        [[nodiscard]]
        loop_t loop() const {
            return loop_t::borrow(raw<raw_t>()->loop);
        }

        [[nodiscard]]
        const char *host() const {
            return raw<raw_t>()->host;
        }

        [[nodiscard]]
        const char *service() const {
            return raw<raw_t>()->service;
        }

    public:
        class callback_t : public req_callback_t<raw_t, int, const char *, const char *> {
        public:
            using self = callback_t;
            using supper = req_callback_t<raw_t, int, const char *, const char *>;

            using promise_t = uvcxx::promise<const char *, const char *>;
            using promise_cast_t = uvcxx::promise_cast<promise_t, raw_t *, int, const char *, const char *>;

            promise_cast_t promise;

            explicit callback_t(const getnameinfo_t &req)
                    : supper(req), promise([](uv_getnameinfo_t *req, int, const char *hostname, const char *service) {
                return std::make_tuple(hostname, service);
            }) {
            }

            typename promise_cast_t::supper &proxy() noexcept final {
                return promise;
            }

            void finalize(raw_t *, int, const char *, const char *) noexcept final {
            }

            int check(raw_t *, int status, const char *, const char *) noexcept final {
                return status;
            }
        };
    };

    [[nodiscard]]
    inline uvcxx::promise<const char *, const char *> getnameinfo(
            const loop_t &loop, const getnameinfo_t &req,
            const sockaddr *addr, int flags) {
        auto *data = new getnameinfo_t::callback_t(req);
        uvcxx::defer_delete delete_data(data);

        auto err = uv_getnameinfo(loop, req, getnameinfo_t::callback_t::raw_callback, addr, flags);
        if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);

        delete_data.release();
        return data->promise.promise();
    }

    [[nodiscard]]
    inline uvcxx::promise<const char *, const char *> getnameinfo(
            const sockaddr *addr, int flags) {
        return getnameinfo(default_loop(), {}, addr, flags);
    }

    [[nodiscard]]
    inline uvcxx::promise<const char *, const char *> getnameinfo(
            const getnameinfo_t &req,
            const sockaddr *addr, int flags) {
        return getnameinfo(default_loop(), req, addr, flags);
    }

    [[nodiscard]]
    inline uvcxx::promise<const char *, const char *> getnameinfo(
            const loop_t &loop,
            const sockaddr *addr, int flags) {
        return getnameinfo(loop, {}, addr, flags);
    }
}

#endif //LIBUVCXX_GETNAMEINFO_H
