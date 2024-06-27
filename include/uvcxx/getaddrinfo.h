//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_GETADDRINFO_H
#define LIBUVCXX_GETADDRINFO_H

#include "cxx/string.h"

#include "loop.h"
#include "req.h"

namespace uv {
    class getaddrinfo_t : public inherit_req_t<uv_getaddrinfo_t, req_t> {
    public:
        using self = getaddrinfo_t;
        using supper = inherit_req_t<uv_getaddrinfo_t, req_t>;

        UVCXX_NODISCARD
        loop_t loop() const {
            return loop_t::borrow(raw<raw_t>()->loop);
        }

        UVCXX_NODISCARD
        struct addrinfo *addrinfo() const {
            return raw<raw_t>()->addrinfo;
        }

    public:
        class callback_t : public req_callback_t<raw_t, int, ::addrinfo *> {
        public:
            using self = callback_t;
            using supper = req_callback_t<raw_t, int, ::addrinfo *>;

            using promise_t = uvcxx::promise<::addrinfo *>;
            using promise_cast_t = uvcxx::promise_cast<uvcxx::promise<::addrinfo *>, raw_t *, int, ::addrinfo *>;

            promise_cast_t promise;

            explicit callback_t(const getaddrinfo_t &req)
                    : supper(req), promise([](raw_t *, int, ::addrinfo *ai) {
                return ai;
            }) {
            }

            typename promise_cast_t::supper &proxy() UVCXX_NOEXCEPT final {
                return promise;
            }

            void finalize(raw_t *, int, ::addrinfo *ai) UVCXX_NOEXCEPT final {
                uv_freeaddrinfo(ai);
            }

            int check(raw_t *, int status, ::addrinfo *) UVCXX_NOEXCEPT final {
                return status;
            }
        };
    };

    inline void freeaddrinfo(addrinfo *ai) { uv_freeaddrinfo(ai); }

#if UVCXX_SATISFY_VERSION(1, 3, 0)

    inline int getaddrinfo(std::nullptr_t, getaddrinfo_t &req,
                           uvcxx::string node, uvcxx::string service, const addrinfo *hints,
                           std::nullptr_t) {
        UVCXX_PROXY(uv_getaddrinfo(nullptr, req, nullptr, node, service, hints));
    }

#endif

    UVCXX_NODISCARD
    inline uvcxx::promise<addrinfo *> getaddrinfo(
            const loop_t &loop, const getaddrinfo_t &req,
            uvcxx::string node, uvcxx::string service, const addrinfo *hints = nullptr) {
        auto *data = new getaddrinfo_t::callback_t(req);
        uvcxx::defer_delete<getaddrinfo_t::data_t> delete_data(data);

        UVCXX_APPLY(uv_getaddrinfo(loop, req, getaddrinfo_t::callback_t::raw_callback, node, service, hints), nullptr);

        delete_data.release();
        ((uv_getaddrinfo_t *) req)->data = data;
        return data->promise.promise();
    }

    UVCXX_NODISCARD
    inline uvcxx::promise<addrinfo *> getaddrinfo(
            uvcxx::string node, uvcxx::string service, const addrinfo *hints = nullptr) {
        return getaddrinfo(default_loop(), {}, node, service, hints);
    }

    UVCXX_NODISCARD
    inline uvcxx::promise<addrinfo *> getaddrinfo(
            const getaddrinfo_t &req,
            uvcxx::string node, uvcxx::string service, const addrinfo *hints = nullptr) {
        return getaddrinfo(default_loop(), req, node, service, hints);
    }

    UVCXX_NODISCARD
    inline uvcxx::promise<addrinfo *> getaddrinfo(
            const loop_t &loop,
            uvcxx::string node, uvcxx::string service, const addrinfo *hints = nullptr) {
        return getaddrinfo(loop, {}, node, service, hints);
    }
}

#endif //LIBUVCXX_GETADDRINFO_H
