//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_WORK_H
#define LIBUVCXX_WORK_H

#include "loop.h"
#include "req.h"

namespace uv {
    class work_t : public req_extend_t<uv_work_t, req_t> {
    public:
        using self = work_t;
        using supper = req_extend_t<uv_work_t, req_t>;

        class data_t : public req_callback_t<uv_work_t, int> {
        public:
            using self = data_t;
            using supper = req_callback_t<uv_work_t, int>;

            using promise_t = uvcxx::promise<>;
            using promise_cast_t = uvcxx::promise_cast<promise_t, uv_work_t *, int>;

            promise_cast_t promise;

            explicit data_t(const work_t &req)
                    : supper(req), promise([](uv_work_t *, int) { return std::make_tuple(); }) {
            }

            typename promise_cast_t::supper &proxy() noexcept final { return promise; }

            void finalize(uv_work_t *, int) noexcept final {}

            int check(uv_work_t *, int status) noexcept final { return status; }
        };

        [[nodiscard]]
        loop_t loop() const {
            return loop_t::borrow(raw<raw_t>()->loop);
        }
    };

    namespace inner::work {
        static void raw_work_callback(uv_work_t *) {}
    }

    [[nodiscard]]
    inline uvcxx::promise<> queue_work(const loop_t &loop, const work_t &req) {
        auto data = new work_t::data_t(req);
        uvcxx::defer_delete delete_data(data);

        auto err = uv_queue_work(
                loop, req,
                inner::work::raw_work_callback,
                work_t::data_t::raw_callback);
        if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);

        delete_data.release();
        return data->promise.promise();
    }

    [[nodiscard]]
    inline uvcxx::promise<> queue_work() {
        return queue_work(default_loop(), {});
    }

    [[nodiscard]]
    inline uvcxx::promise<> queue_work(const loop_t &loop) {
        return queue_work(loop, {});
    }

    [[nodiscard]]
    inline uvcxx::promise<> queue_work(const work_t &req) {
        return queue_work(default_loop(), req);
    }
}

#endif //LIBUVCXX_WORK_H
