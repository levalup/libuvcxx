//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_SIGNAL_H
#define LIBUVCXX_SIGNAL_H

#include "handle.h"

namespace uv {
    class signal_t : public handle_extend_t<uv_signal_t, handle_t> {
    public:
        using self = signal_t;
        using supper = handle_extend_t<uv_signal_t, handle_t>;

        class data_t : supper::data_t {
        public:
            using self = data_t;
            using supper = supper::data_t;

            uvcxx::callback_emitter<int> start_cb;

            explicit data_t(signal_t &handle)
                    : supper(handle) {
                handle.watch(start_cb);
            }

            void close() noexcept final {
                // finally at close, make queue safe
                start_cb.finalize();
            }
        };

        signal_t() : self(default_loop()) {}

        explicit signal_t(const loop_t &loop) {
            (void) uv_signal_init(loop, *this);
            // data will be deleted in close action
            set_data(new data_t(*this));
        }

        [[nodiscard]]
        int signum() const {
            return raw<raw_t>()->signum;
        }

        [[nodiscard]]
        uvcxx::callback<int> start(uint64_t timeout, int signum) {
            auto err = uv_signal_start(*this, raw_callback, signum);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);
            auto data = get_data<data_t>();
            return data->start_cb.callback();
        }

        [[nodiscard]]
        uvcxx::callback<int> start_oneshot(uint64_t timeout, int signum) {
            auto err = uv_signal_start_oneshot(*this, raw_callback, signum);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);
            auto data = get_data<data_t>();
            return data->start_cb.callback();
        }

        int stop() {
            auto err = uv_signal_stop(*this);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        operator raw_t *() { return raw<raw_t>(); }

        operator raw_t *() const { return raw<raw_t>(); }

    private:
        static void raw_callback(raw_t *handle, int signum) {
            auto data = (data_t *) (handle->data);
            data->start_cb.emit(signum);
        }
    };
}

#endif //LIBUVCXX_SIGNAL_H
