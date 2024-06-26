//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_CHECK_H
#define LIBUVCXX_CHECK_H

#include "handle.h"

namespace uv {
    class check_t : public inherit_handle_t<uv_check_t, handle_t> {
    public:
        using self = check_t;
        using supper = inherit_handle_t<uv_check_t, handle_t>;

        check_t() : self(default_loop()) {}

        explicit check_t(const loop_t &loop) {
            set_data(new data_t(*this));    //< data will be deleted in close action
            (void) uv_check_init(loop, *this);
            _attach_close_();
        }

        UVCXX_NODISCARD
        uvcxx::callback<> start() {
            auto err = uv_check_start(*this, raw_callback);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);
            _detach_();
            return get_data<data_t>()->start_cb.callback();
        }

        void stop() {
            (void) uv_check_stop(*this);
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

            explicit data_t(check_t &handle)
                    : supper::data_t(handle) {
                handle.watch(start_cb);
            }

            ~data_t() override {
                start_cb.finalize();
            }
        };
    };
}

#endif //LIBUVCXX_CHECK_H
