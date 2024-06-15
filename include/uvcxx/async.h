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

        [[nodiscard]]
        uvcxx::callback<> init() {
            return init(default_loop());
        }

        [[nodiscard]]
        uvcxx::callback<> init(const loop_t &loop) {
            auto data = get_data<data_t>();
            auto err = uv_async_init(loop, *this, raw_callback);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);;
            return data->send_cb.callback();
        }

        int send() {
            auto err = uv_async_send(*this);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

    private:
        static void raw_callback(raw_t *handle) {
            auto data = (data_t * )(handle->data);
            data->send_cb.emit();
        }

        class data_t : supper::data_t {
        public:
            using self = data_t;
            using supper = supper::data_t;

            uvcxx::callback_emitter<> send_cb;

            explicit data_t(async_t &handle)
                    : supper(handle) {
                handle.watch(send_cb);
            }

            void close() noexcept final {
                send_cb.finalize();
            }
        };
    };
}

#endif //LIBUVCXX_ASYNC_H
