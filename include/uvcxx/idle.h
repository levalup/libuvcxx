//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_IDLE_H
#define LIBUVCXX_IDLE_H

#include "handle.h"

namespace uv {
    class idle_t : public inherit_handle_t<uv_idle_t, handle_t> {
    public:
        using self = idle_t;
        using supper = inherit_handle_t<uv_idle_t, handle_t>;

        idle_t() : self(default_loop()) {}

        explicit idle_t(const loop_t &loop) {
            set_data(new data_t(*this));    //< data will be deleted in close action
            (void) uv_idle_init(loop, *this);
            _attach_close_();
        }

        UVCXX_NODISCARD
        uvcxx::callback<> start() {
            UVCXX_APPLY(uv_idle_start(*this, raw_callback), nullptr);
            _detach_();
            return get_data<data_t>()->start_cb.callback();
        }

        void stop() {
            (void) uv_idle_stop(*this);
            _attach_close_();
        }

    private:
        static void raw_callback(raw_t *handle) {
            auto data = (data_t *) (handle->data);
            data->start_cb.emit();
        }

        class data_t : supper::data_t {
        public:
            uvcxx::callback_emitter<> start_cb;

            explicit data_t(idle_t &handle)
                    : supper::data_t(handle) {
                handle.watch(start_cb);
            }

            ~data_t() override {
                start_cb.finalize();
            }
        };
    };
}

#endif //LIBUVCXX_IDLE_H
