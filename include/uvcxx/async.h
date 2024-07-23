//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_ASYNC_H
#define LIBUVCXX_ASYNC_H

#include "handle.h"

namespace uv {
    class async_t : public inherit_handle_t<uv_async_t, handle_t> {
    public:
        using self = async_t;
        using supper = inherit_handle_t<uv_async_t, handle_t>;

        async_t() {
            set_data(new data_t(*this));    //< data will be deleted in close action
        }

        self &detach() {
            _detach_();
            return *this;
        }

        UVCXX_NODISCARD
        uvcxx::callback<> callback() {
            return get_data<data_t>()->send_cb.callback();;
        }

        UVCXX_NODISCARD
        uvcxx::attached_callback<> init(const loop_t &loop) {
            UVCXX_APPLY(uv_async_init(loop, *this, raw_callback), nullptr);
            _initialized_();
            return {*this, callback()};
        }

        UVCXX_NODISCARD
        uvcxx::attached_callback<> init() {
            return init(default_loop());
        }

        int send() {
            UVCXX_PROXY(uv_async_send(*this));
        }

    private:
        static void raw_callback(raw_t *handle) {
            auto data = (data_t * )(handle->data);
            data->send_cb.emit();
        }

        class data_t : supper::data_t {
        public:
            uvcxx::callback_emitter<> send_cb;

            explicit data_t(async_t &handle)
                    : supper::data_t(handle) {
                handle.watch(send_cb);
            }

            ~data_t() override {
                send_cb.finalize();
            }
        };
    };
}

#endif //LIBUVCXX_ASYNC_H
