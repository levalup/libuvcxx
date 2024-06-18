//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_SIGNAL_H
#define LIBUVCXX_SIGNAL_H

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
        }

        [[nodiscard]]
        int signum() const {
            return raw<raw_t>()->signum;
        }

        [[nodiscard]]
        uvcxx::callback<int> start(int signum) {
            auto err = uv_signal_start(*this, raw_callback, signum);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);
            auto data = get_data<data_t>();
            return data->start_cb.callback();
        }

#if UVCXX_SATISFY_VERSION(1, 12, 0)

        [[nodiscard]]
        uvcxx::callback<int> start_oneshot(int signum) {
            auto err = uv_signal_start_oneshot(*this, raw_callback, signum);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);
            auto data = get_data<data_t>();
            return data->start_cb.callback();
        }

#endif

        int stop() {
            auto err = uv_signal_stop(*this);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

    private:
        static void raw_callback(raw_t *handle, int signum) {
            auto data = (data_t *) (handle->data);
            data->start_cb.emit(signum);
        }

        class data_t : supper::data_t {
        public:
            uvcxx::callback_emitter<int> start_cb;

            explicit data_t(signal_t &handle)
                    : supper::data_t(handle) {
                handle.watch(start_cb);
            }

            void close() noexcept final {
                start_cb.finalize();
            }
        };
    };
}

#endif //LIBUVCXX_SIGNAL_H
