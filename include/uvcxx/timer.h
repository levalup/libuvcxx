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

        class data_t : supper::data_t {
        public:
            using self = data_t;
            using supper = supper::data_t;

            uvcxx::callback_emitter<> start_cb;

            explicit data_t(timer_t &handle)
                    : supper(handle) {
                handle.watch(start_cb);
            }

            void close() noexcept final {
                // finally at close, make queue safe
                start_cb.finalize();
            }
        };

        timer_t() : self(default_loop()) {}

        explicit timer_t(const loop_t &loop) {
            (void) uv_timer_init(loop, *this);
            // data will be deleted in close action
            set_data(new data_t(*this));
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

        [[nodiscard]]
        uint64_t get_due_in() const {
            return uv_timer_get_repeat(*this);
        }

        operator raw_t *() { return raw<raw_t>(); }

        operator raw_t *() const { return raw<raw_t>(); }

    private:
        static void raw_callback(raw_t *handle) {
            auto data = (data_t *) (handle->data);
            data->start_cb.emit();
        }
    };
}

#endif //LIBUVCXX_TIMER_H
