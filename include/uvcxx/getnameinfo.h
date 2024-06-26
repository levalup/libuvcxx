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

        UVCXX_NODISCARD
        loop_t loop() const {
            return loop_t::borrow(raw<raw_t>()->loop);
        }

        UVCXX_NODISCARD
        const char *host() const {
            return raw<raw_t>()->host;
        }

        UVCXX_NODISCARD
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
                    : supper(req), promise([](uv_getnameinfo_t *, int, const char *hostname, const char *service) {
                return std::make_tuple(hostname, service);
            }) {
            }

            typename promise_cast_t::supper &proxy() UVCXX_NOEXCEPT final {
                return promise;
            }

            void finalize(raw_t *, int, const char *, const char *) UVCXX_NOEXCEPT final {
            }

            int check(raw_t *, int status, const char *, const char *) UVCXX_NOEXCEPT final {
                return status;
            }
        };
    };

#if UVCXX_SATISFY_VERSION(1, 3, 0)

    inline int getnameinfo(std::nullptr_t, getnameinfo_t &req,
                           const sockaddr *addr, int flags,
                           std::nullptr_t) {
        UVCXX_PROXY(uv_getnameinfo(nullptr, req, nullptr, addr, flags));
    }

#endif

    UVCXX_NODISCARD
    inline uvcxx::promise<const char *, const char *> getnameinfo(
            const loop_t &loop, const getnameinfo_t &req,
            const sockaddr *addr, int flags) {
        auto *data = new getnameinfo_t::callback_t(req);
        uvcxx::defer_delete<getnameinfo_t::data_t> delete_data(data);

        UVCXX_APPLY(uv_getnameinfo(loop, req, getnameinfo_t::callback_t::raw_callback, addr, flags), nullptr);

        delete_data.release();
        ((uv_getnameinfo_t *) req)->data = data;
        return data->promise.promise();
    }

    UVCXX_NODISCARD
    inline uvcxx::promise<const char *, const char *> getnameinfo(
            const sockaddr *addr, int flags) {
        return getnameinfo(default_loop(), {}, addr, flags);
    }

    UVCXX_NODISCARD
    inline uvcxx::promise<const char *, const char *> getnameinfo(
            const getnameinfo_t &req,
            const sockaddr *addr, int flags) {
        return getnameinfo(default_loop(), req, addr, flags);
    }

    UVCXX_NODISCARD
    inline uvcxx::promise<const char *, const char *> getnameinfo(
            const loop_t &loop,
            const sockaddr *addr, int flags) {
        return getnameinfo(loop, {}, addr, flags);
    }
}

#endif //LIBUVCXX_GETNAMEINFO_H
