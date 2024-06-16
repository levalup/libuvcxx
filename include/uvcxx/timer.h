//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_TIMER_H
#define LIBUVCXX_TIMER_H

#include "handle.h"

namespace uv {
    class timer_t : public inherit_handle_t<uv_timer_t, handle_t> {
    public:
        using self = timer_t;
        using supper = inherit_handle_t<uv_timer_t, handle_t>;

        timer_t() : self(default_loop()) {}

        explicit timer_t(const loop_t &loop) {
            set_data(new data_t(*this));    //< data will be deleted in close action
            (void) uv_timer_init(loop, *this);
        }

        [[nodiscard]]
        uvcxx::callback<> start(uint64_t timeout, uint64_t repeat) {
            auto err = uv_timer_start(*this, raw_callback, timeout, repeat);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);
            auto data = get_data<data_t>();
            return data->start_cb.callback();
        }

        int stop() {
            auto err = uv_timer_stop(*this);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        int again() {
            auto err = uv_timer_again(*this);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        void set_repeat(uint64_t repeat) {
            uv_timer_set_repeat(*this, repeat);
        }

        [[nodiscard]]
        uint64_t get_repeat() const {
            return uv_timer_get_repeat(*this);
        }

#if UVCXX_SATISFY_VERSION(1, 40, 0)

        [[nodiscard]]
        uint64_t get_due_in() const {
            return uv_timer_get_repeat(*this);
        }

#endif

    private:
        static void raw_callback(raw_t *handle) {
            auto data = (data_t *) (handle->data);
            data->start_cb.emit();
        }

        class data_t : supper::data_t {
        public:
            uvcxx::callback_emitter<> start_cb;

            explicit data_t(timer_t &handle)
                    : supper::data_t(handle) {
                handle.watch(start_cb);
            }

            void close() noexcept final {
                start_cb.finalize();
            }
        };
    };
}

#endif //LIBUVCXX_TIMER_H
