//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_SIGNAL_H
#define LIBUVCXX_SIGNAL_H

#include "utils/promise2.h"

#include "handle.h"

namespace uv {
    class signal_t : public inherit_handle_t<uv_signal_t, handle_t> {
    public:
        using self = signal_t;
        using supper = inherit_handle_t<uv_signal_t, handle_t>;

        signal_t() : self(default_loop()) {}

        explicit signal_t(const loop_t &loop) {
            set_data(new data_t(*this));    //< data will be deleted in close action
            (void) uv_signal_init(loop, *this);
            _initialized_();
        }

        self &detach() {
            _detach_();
            return *this;
        }

        UVCXX_NODISCARD
        int signum() const {
            return raw<raw_t>()->signum;
        }

        UVCXX_NODISCARD
        uvcxx::callback<int> callback() {
            return get_data<data_t>()->start_cb.callback();
        }

        UVCXX_NODISCARD
        uvcxx::callback2<int> start(int signum) {
            UVCXX_APPLY(uv_signal_start(*this, raw_callback, signum), nullptr);
            return {*this, callback()};
        }

#if UVCXX_SATISFY_VERSION(1, 12, 0)

        UVCXX_NODISCARD
        uvcxx::promise2<int> start_oneshot(int signum) {
            UVCXX_APPLY(uv_signal_start_oneshot(*this, raw_oneshot_callback, signum), nullptr);
            return {*this, get_data<data_t>()->start_oneshot_cb.promise()};
        }

#endif

        int stop() {
            UVCXX_APPLY(uv_signal_stop(*this), status);
            return 0;
        }

    private:
        static void raw_callback(raw_t *handle, int signum) {
            auto data = (data_t *) (handle->data);
            data->start_cb.emit(signum);
        }

        static void raw_oneshot_callback(raw_t *handle, int signum) {
            auto data = (data_t *) (handle->data);
            data->start_oneshot_cb.resolve(signum);
            data->start_oneshot_cb.finalize();
            data->start_oneshot_cb.promise().then(nullptr).except(nullptr).finally(nullptr);
        }

        class data_t : supper::data_t {
        public:
            uvcxx::callback_emitter<int> start_cb;
            uvcxx::promise_emitter<int> start_oneshot_cb;

            explicit data_t(signal_t &handle)
                    : supper::data_t(handle) {
                handle.watch(start_cb);
            }

            ~data_t() override {
                start_cb.finalize();
            }
        };
    };
}

#endif //LIBUVCXX_SIGNAL_H
