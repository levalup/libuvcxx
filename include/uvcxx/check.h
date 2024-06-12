//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_CHECK_H
#define LIBUVCXX_CHECK_H

#include "handle.h"

namespace uv {
    class check_t : public handle_extend_t<uv_check_t, handle_t> {
    public:
        using self = check_t;
        using supper = handle_extend_t<uv_check_t, handle_t>;

        class data_t : supper::data_t {
        public:
            using self = data_t;
            using supper = supper::data_t;

            uvcxx::callback_emitter<> start_cb;

            explicit data_t(check_t &handle)
                    : supper(handle) {
                handle.watch(start_cb);
            }

            void close() noexcept final {
                // finally at close, make queue safe
                start_cb.finalize();
            }
        };

        check_t() : self(default_loop()) {}

        explicit check_t(const loop_t &loop) {
            (void) uv_check_init(loop, *this);
            // data will be deleted in close action
            set_data(new data_t(*this));
        }

        [[nodiscard]]
        uvcxx::callback<> start() {
            auto err = uv_check_start(*this, raw_callback);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);
            auto data = get_data<data_t>();
            return data->start_cb.callback();
        }

        void stop() {
            (void) uv_check_stop(*this);
        }

    private:
        static void raw_callback(raw_t *handle) {
            auto data = (data_t *) (handle->data);
            data->start_cb.emit();
        }
    };
}

#endif //LIBUVCXX_CHECK_H
