//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_GETADDRINFO_H
#define LIBUVCXX_GETADDRINFO_H

#include "loop.h"
#include "req.h"

namespace uv {
    class getaddrinfo_t : public req_extend_t<uv_getaddrinfo_t, req_t> {
    public:
        using self = getaddrinfo_t;
        using supper = req_extend_t<uv_getaddrinfo_t, req_t>;

        [[nodiscard]]
        loop_t loop() const {
            return loop_t::borrow(raw<raw_t>()->loop);
        }

        [[nodiscard]]
        struct addrinfo *addrinfo() const {
            return raw<raw_t>()->addrinfo;
        }
    };

    namespace inner::getaddrinfo {
        using cxx_req_t = getaddrinfo_t;
        using raw_req_t = cxx_req_t::raw_t;

        template<typename... T>
        class callback_t : public req_callback_t<raw_req_t, int, addrinfo *> {
        public:
            using self = callback_t;
            using supper = req_callback_t<raw_req_t, int, addrinfo *>;

            using promise_t = uvcxx::promise<T...>;
            using promise_cast_t = uvcxx::promise_cast<uvcxx::promise<T...>, raw_req_t *, int, addrinfo *>;

            promise_cast_t promise;

            explicit callback_t(const cxx_req_t &req, typename promise_cast_t::wrapper_t wrapper)
                    : supper(req), promise(promise_t(), std::move(wrapper)) {
            }

            typename promise_cast_t::supper &proxy() noexcept final {
                return promise;
            }

            void finalize(raw_req_t *, int, addrinfo *ai) noexcept final {
                uv_freeaddrinfo(ai);
            }

            int check(raw_req_t *, int status, addrinfo *) noexcept final {
                return status;
            }
        };
    }

    [[nodiscard]]
    inline uvcxx::promise<addrinfo *> getaddrinfo(
            const loop_t &loop, const getaddrinfo_t &req,
            const char *node, const char *service, const addrinfo *hints = nullptr) {
        using raw_req_t = inner::getaddrinfo::raw_req_t;
        using callback_t = inner::getaddrinfo::callback_t<addrinfo *>;
        auto *data = new callback_t(
                req, [](raw_req_t *, int, addrinfo *res) {
                    return res;
                });
        uvcxx::defer delete_data(std::default_delete<callback_t>(), data);

        auto err = uv_getaddrinfo(loop, data->req<raw_req_t>(), callback_t::raw_callback, node, service, hints);
        if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);
        delete_data.release();
        return data->promise.promise();
    }

    [[nodiscard]]
    inline uvcxx::promise<addrinfo *> getaddrinfo(
            const char *node, const char *service, const addrinfo *hints = nullptr) {
        return getaddrinfo(default_loop(), {}, node, service, hints);
    }

    [[nodiscard]]
    inline uvcxx::promise<addrinfo *> getaddrinfo(
            const getaddrinfo_t &req,
            const char *node, const char *service, const addrinfo *hints = nullptr) {
        return getaddrinfo(default_loop(), req, node, service, hints);
    }

    [[nodiscard]]
    inline uvcxx::promise<addrinfo *> getaddrinfo(
            const loop_t &loop,
            const char *node, const char *service, const addrinfo *hints = nullptr) {
        return getaddrinfo(loop, {}, node, service, hints);
    }
}

#endif //LIBUVCXX_GETADDRINFO_H
