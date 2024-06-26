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
            _attach_close_();
        }

        UVCXX_NODISCARD
        int signum() const {
            return raw<raw_t>()->signum;
        }

        UVCXX_NODISCARD
        uvcxx::callback<int> start(int signum) {
            auto err = uv_signal_start(*this, raw_callback, signum);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);
            _detach_();
            return get_data<data_t>()->start_cb.callback();
        }

#if UVCXX_SATISFY_VERSION(1, 12, 0)

        UVCXX_NODISCARD
        uvcxx::callback<int> start_oneshot(int signum) {
            auto err = uv_signal_start_oneshot(*this, raw_callback, signum);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);

            _detach_();

            auto attachment = *this;
            return get_data<data_t>()->start_cb.callback().finally(nullptr)
                    .finally([attachment]() mutable {
                        finally_recycle_oneshot(attachment);
                    });
        }

#endif

        int stop() {
            auto err = uv_signal_stop(*this);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            _attach_close_();
            return err;
        }

#if UVCXX_SATISFY_VERSION(1, 12, 0)

    private:
        /**
         * recycle grab signal_t.
         * if use_count == 1, close signal, because the `callback` and `signal` can not be reached ever again.
         * if use_count > 1, attach_close and unref_attach, telling other refs that its attached again. and i'm done.
         * @param grab
         */
        static void finally_recycle_oneshot(signal_t &grab) {
            if (grab._attach_count_() == 1) {
                grab.close(nullptr);
            } else {
                grab._attach_close_();
                grab._unref_attach_();
            }
        }

#endif

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

            ~data_t() override {
                start_cb.finalize();
            }
        };
    };
}

#endif //LIBUVCXX_SIGNAL_H
