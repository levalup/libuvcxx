//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_CHECK_H
#define LIBUVCXX_CHECK_H

#include <uv.h>

#include "loop.h"
#include "handle.h"

#include "cxx/callback.h"

namespace uv {
    class check_t : public handle_extend_t<uv_check_t, handle_t> {
    public:
        using self = check_t;
        using supper = handle_extend_t<uv_check_t, handle_t>;

        class data_t : supper::data_t {
        public:
            uvcxx::callback_cast<uvcxx::callback<check_t *>> start_cb;

            // Store the instance of `handle` in `start_cb's wrapper`
            //     to avoid resource release caused by no external reference
            explicit data_t(check_t check)
                : start_cb([check = std::move(check)]() mutable { return &check; }){
            }

            void close() noexcept final {
                // finally at close, make queue safe
                start_cb.finally();
            }
        };

        check_t() : self(default_loop()) {}

        explicit check_t(loop_t loop) {
            uv_check_init(loop, *this);
            // data will be deleted in close action
            set_data(new data_t(*this));
        }

        uvcxx::callback<check_t*> start() {
            auto err = uv_check_start(*this, raw_callback);
            if (err < 0) throw uvcxx::exception(err);
            auto data = (data_t *)(get_data());
            return data->start_cb.callback();
        }

        void stop() {
            (void) uv_check_stop(*this);
        }

    private:
        static void raw_callback(raw_t *handle) {
            auto data = (data_t *)(handle->data);
            data->start_cb.emit();
        }
    };
}

#endif //LIBUVCXX_CHECK_H
