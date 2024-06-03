//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_FS_H
#define LIBUVCXX_FS_H

#include <functional>
#include <utility>

#include <uv.h>

#include "loop.h"
#include "req.h"

#include "cxx/memory.h"
#include "cxx/defer.h"
#include "cxx/promise.h"

namespace uv {
    class fs_t : public req_extend_t<uv_fs_t, req_t> {
    public:
        using self = fs_t;
        using supper = req_extend_t<uv_fs_t, req_t>;
    };

    namespace fs {
        namespace inner {
            using cxx_req_t = fs_t;
            using raw_req_t = cxx_req_t::raw_t;

            template<typename... T>
            class callback_t : public req_callback_t<raw_req_t> {
            public:
                using self = callback_t;

                using promise_t = uvcxx::promise<T...>;
                using promise_cast_t = uvcxx::promise_cast<uvcxx::promise<T...>, raw_req_t *>;

                // store the instance of `req` to avoid resource release caused by no external reference
                cxx_req_t cxx_req;
                promise_cast_t promise;

                explicit callback_t(cxx_req_t req, typename promise_cast_t::wrapper_t wrapper)
                        : cxx_req(std::move(req)), promise(promise_t(), wrapper) {
                    cxx_req.set_data(this);
                }

                uvcxx::promise_proxy<raw_req_t *> &proxy() noexcept final {
                    return promise;
                }

                void finalize(raw_req_t *req) noexcept final {
                    uv_fs_req_cleanup(req);
                }

                int check(raw_req_t *req) noexcept final {
                    return (int) req->result;
                }
            };

            template<typename... T>
            class invoker {
            public:
                template<typename FUNC, typename ...ARGS,
                        typename=typename std::enable_if_t<std::is_integral_v<
                                std::invoke_result_t<FUNC,
                                        loop_t::raw_t *, raw_req_t *, ARGS..., decltype(callback_t<T...>::raw_callback)>
                        >>>
                uvcxx::promise<T...> operator()(
                        typename callback_t<T...>::promise_cast_t::wrapper_t wrapper,
                        FUNC func, loop_t loop, cxx_req_t req, ARGS &&...args) const {
                    auto *data = new callback_t<T...>(std::move(req), std::move(wrapper));
                    uvcxx::defer delete_data(std::default_delete<callback_t<T...>>(), data);

                    auto err = std::invoke(
                            func,
                            (loop_t::raw_t *) loop,
                            (cxx_req_t::raw_t *) data->cxx_req,
                            std::forward<ARGS>(args)...,
                            callback_t<T...>::raw_callback);

                    if (err < 0) throw uvcxx::exception(err);
                    delete_data.release();
                    return data->promise.promise();
                }
            };
        }

        inline uvcxx::promise<uv_file> open(loop_t loop, fs_t fs, const char *path, int flags, int mode) {
            return inner::invoker<uv_file>()(
                    [](inner::raw_req_t *req) { return (uv_file) req->result; },
                    uv_fs_open, std::move(loop), std::move(fs), path, flags, mode);
        }

        inline uvcxx::promise<uv_file> open(const char *path, int flags, int mode) {
            return open(default_loop(), inner::cxx_req_t(), path, flags, mode);
        }

        inline uvcxx::promise<uv_file> open(loop_t loop, const char *path, int flags, int mode) {
            return open(std::move(loop), inner::cxx_req_t(), path, flags, mode);
        }

        inline uvcxx::promise<uv_file> open(fs_t fs, const char *path, int flags, int mode) {
            return open(default_loop(), std::move(fs), path, flags, mode);
        }

        inline uvcxx::promise<int> close(loop_t loop, fs_t fs, uv_file file) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_close, std::move(loop), std::move(fs), file);
        }

        inline uvcxx::promise<int> close(uv_file file) {
            return close(default_loop(), inner::cxx_req_t(), file);
        }

        inline uvcxx::promise<int> close(loop_t loop, uv_file file) {
            return close(std::move(loop), inner::cxx_req_t(), file);
        }

        inline uvcxx::promise<int> close(fs_t fs, uv_file file) {
            return close(default_loop(), std::move(fs), file);
        }
    }
}


#endif //LIBUVCXX_FS_H
