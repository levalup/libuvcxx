//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_TIMER_H
#define LIBUVCXX_TIMER_H

#include <uv.h>

#include "loop.h"
#include "handle.h"

#include "cxx/callback.h"

namespace uv {
    class timer_t : public handle_extend_t<uv_timer_t, handle_t> {
    public:
        using self = timer_t;
        using supper = handle_extend_t<uv_timer_t, handle_t>;

        class data_t : supper::data_t {
        public:
            uvcxx::callback_cast<uvcxx::callback<timer_t *>> start_cb;

            // Store the instance of `handle` in `start_cb's wrapper`
            //     to avoid resource release caused by no external reference
            explicit data_t(timer_t timer)
                : start_cb([timer = std::move(timer)]() mutable { return &timer; }){
            }

            void close() noexcept final {
                // finally at close, make queue safe
                start_cb.finally();
            }
        };

        timer_t() : self(default_loop()) {}

        explicit timer_t(loop_t loop) {
            uv_timer_init(loop, *this);
            // data will be deleted in close action
            set_data(new data_t(*this));
        }

        uvcxx::callback<timer_t*> start(uint64_t timeout, uint64_t repeat) {
            auto err = uv_timer_start(*this, raw_callback, timeout, repeat);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);
            auto data = (data_t *)(get_data());
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

    private:
        static void raw_callback(raw_t *handle) {
            auto data = (data_t *)(handle->data);
            data->start_cb.emit();
        }
    };
}

#endif //LIBUVCXX_TIMER_H
