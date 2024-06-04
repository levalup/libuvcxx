//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_PREPARE_H
#define LIBUVCXX_PREPARE_H

#include <uv.h>

#include "loop.h"
#include "handle.h"

#include "cxx/callback.h"

namespace uv {
    class prepare_t : public handle_extend_t<uv_prepare_t, handle_t> {
    public:
        using self = prepare_t;
        using supper = handle_extend_t<uv_prepare_t, handle_t>;

        class data_t : supper::data_t {
        public:
            uvcxx::callback_cast<uvcxx::callback<prepare_t *>> start_cb;

            // Store the instance of `handle` in `start_cb's wrapper`
            //     to avoid resource release caused by no external reference
            explicit data_t(prepare_t prepare)
                : start_cb([prepare = std::move(prepare)]() mutable { return &prepare; }){
            }

            void close() noexcept final {
                // finally at close, make queue safe
                start_cb.finally();
            }
        };

        prepare_t() : self(default_loop()) {}

        explicit prepare_t(loop_t loop) {
            uv_prepare_init(loop, *this);
            // data will be deleted in close action
            set_data(new data_t(*this));
        }

        uvcxx::callback<prepare_t*> start() {
            auto err = uv_prepare_start(*this, raw_callback);
            if (err < 0) throw uvcxx::exception(err);
            auto data = (data_t *)(get_data());
            return data->start_cb.callback();
        }

        void stop() {
            (void) uv_prepare_stop(*this);
        }

    private:
        static void raw_callback(raw_t *handle) {
            auto data = (data_t *)(handle->data);
            data->start_cb.emit();
        }
    };
}

#endif //LIBUVCXX_PREPARE_H
