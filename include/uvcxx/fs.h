//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_FS_H
#define LIBUVCXX_FS_H

#include <functional>

#include <uv.h>

#include "cxx/memory.h"
#include "cxx/defer.h"
#include "cxx/promise.h"

#include "./loop.h"

namespace uv::fs {
    using raw_req = uv_fs_t;
    namespace {
        auto req_cleanup = uv_fs_req_cleanup;
    }

    class request {
    public:
        raw_req req{};
        uvcxx::promise_proxy<raw_req *> *proxy{};

        virtual ~request() = default;
    };

    template <typename T>
    class promise_request : public request {
    public:
        uvcxx::promise<T> promise;

        explicit promise_request(typename uvcxx::promise_cast<T, raw_req *>::wrapper_t wrapper) {
            proxy = new uvcxx::promise_cast<T, raw_req *>(promise, std::move(wrapper));
        }

        ~promise_request() override {
            delete proxy;
        }
    };

    static void raw_callback(raw_req *req) {
        using namespace uvcxx;

        auto data = (request*)req->data;
        defer delete_data(std::default_delete<request>(), data);
        defer cleanup(req_cleanup, req);

        defer promise_finally([&]() { data->proxy->finally(); });

        try {
            if (req->result < 0) throw exception(req->result);
            data->proxy->resolve(req);
        } catch (const std::exception &) {
            data->proxy->reject(std::current_exception());
        }
    }

    /**
     * @note The req->result has already checked.
     * @note The function f will be implicitly called at the end.
     */
    uvcxx::promise<uv_file> open(loop &loop, const char *path, int flags, int mode) {
        using namespace uvcxx;

        auto *data = new promise_request<uv_file>([](raw_req *req) {
            return (uv_file)req->result;
        });
        defer delete_req(std::default_delete<request>(), data);
        std::memset(&data->req, 0, sizeof(data->req));
        data->req.data = data;

        auto err = uv_fs_open(loop, &data->req, path, flags, mode, raw_callback);

        if (err < 0) throw exception(err);
        delete_req.release();
        return data->promise;
    }

    uvcxx::promise<int> close(loop &loop, uv_file file) {
        using namespace uvcxx;

        auto *data = new promise_request<int>([](raw_req *req) {
            return (int)req->result;
        });
        defer delete_req(std::default_delete<request>(), data);
        std::memset(&data->req, 0, sizeof(data->req));
        data->req.data = data;

        auto err = uv_fs_close(loop, &data->req, file, raw_callback);

        if (err < 0) throw exception(err);
        delete_req.release();
        return data->promise;
    }
}


#endif //LIBUVCXX_FS_H
