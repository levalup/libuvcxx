//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_GETADDRINFO_H
#define LIBUVCXX_GETADDRINFO_H

#include <functional>
#include <utility>

#include <uv.h>

#include "loop.h"
#include "req.h"

#include "cxx/memory.h"
#include "cxx/defer.h"
#include "cxx/promise.h"

namespace uv {
    class getaddrinfo_t : public req_extend_t<uv_getaddrinfo_t, req_t> {
    public:
        using self = getaddrinfo_t;
        using supper = req_extend_t<uv_getaddrinfo_t, req_t>;

        [[nodiscard]]
        loop_t loop() const {
            auto loop = raw<supper::raw_t>()->loop;
            return loop_t::borrow(loop);
        }
    };

    namespace inner::getaddrinfo {
        using cxx_req_t = getaddrinfo_t;
        using raw_req_t = cxx_req_t::raw_t;

        template<typename... T>
        class callback_t : public req_callback_t<raw_req_t, int, addrinfo *> {
        public:
            using self = callback_t;

            using promise_t = uvcxx::promise<T...>;
            using promise_cast_t = uvcxx::promise_cast<uvcxx::promise<T...>, raw_req_t *, int, addrinfo *>;

            // store the instance of `req` to avoid resource release caused by no external reference
            cxx_req_t cxx_req;
            promise_cast_t promise;

            explicit callback_t(cxx_req_t req, typename promise_cast_t::wrapper_t wrapper)
                    : cxx_req(std::move(req)), promise(promise_t(), wrapper) {
                cxx_req.set_data(this);
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

    inline uvcxx::promise<addrinfo *> getaddrinfo(
            loop_t loop, getaddrinfo_t req,
            const char *node, const char *service, const addrinfo *hints = nullptr) {
        using callback_t = inner::getaddrinfo::callback_t<addrinfo *>;
        auto *data = new callback_t(
                std::move(req), [](inner::getaddrinfo::raw_req_t *, int, addrinfo *res) {
                    return res;
                });
        uvcxx::defer delete_data(std::default_delete<callback_t>(), data);

        auto err = uv_getaddrinfo(loop, data->cxx_req, callback_t::raw_callback, node, service, hints);
        if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);
        delete_data.release();
        return data->promise.promise();
    }

    inline uvcxx::promise<addrinfo *> getaddrinfo(
            const char *node, const char *service, const addrinfo *hints = nullptr) {
        return getaddrinfo(default_loop(), {}, node, service, hints);
    }

    inline uvcxx::promise<addrinfo *> getaddrinfo(
            getaddrinfo_t req,
            const char *node, const char *service, const addrinfo *hints = nullptr) {
        return getaddrinfo(default_loop(), std::move(req), node, service, hints);
    }

    inline uvcxx::promise<addrinfo *> getaddrinfo(
            loop_t loop,
            const char *node, const char *service, const addrinfo *hints = nullptr) {
        return getaddrinfo(std::move(loop), {}, node, service, hints);
    }
}


#endif //LIBUVCXX_GETADDRINFO_H
