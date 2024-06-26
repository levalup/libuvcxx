//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_FS_POLL_H
#define LIBUVCXX_FS_POLL_H

#include "cxx/string.h"
#include "utils/platform.h"

#include "handle.h"

namespace uv {
    class fs_poll_t : public inherit_handle_t<uv_fs_poll_t, handle_t> {
    public:
        using self = fs_poll_t;
        using supper = inherit_handle_t<uv_fs_poll_t, handle_t>;

        fs_poll_t() : self(default_loop()) {}

        explicit fs_poll_t(const loop_t &loop) {
            set_data(new data_t(*this));    //< data will be deleted in close action
            (void) uv_fs_poll_init(loop, *this);
            _attach_close_();
        }

        UVCXX_NODISCARD
        uvcxx::callback<int, const uv_stat_t *, const uv_stat_t *> start(uvcxx::string path, unsigned int interval) {
            auto err = uv_fs_poll_start(*this, raw_callback, path, interval);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);
            _detach_();
            return get_data<data_t>()->start_cb.callback();
        }

        void stop() {
            (void) uv_fs_poll_stop(*this);
            _attach_close_();
        }

        int getpath(char *buffer, size_t *size) {
            return uv_fs_poll_getpath(*this, buffer, size);
        }

    private:
        static void raw_callback(raw_t *handle, int status, const uv_stat_t *prev, const uv_stat_t *curr) {
            auto data = (data_t *) (handle->data);
            data->start_cb.emit(status, prev, curr);
        }

        class data_t : supper::data_t {
        public:
            uvcxx::callback_emitter<int, const uv_stat_t *, const uv_stat_t *> start_cb;

            explicit data_t(fs_poll_t &handle)
                    : supper::data_t(handle) {
                handle.watch(start_cb);
            }

            ~data_t() override {
                start_cb.finalize();
            }
        };
    };
}

#endif //LIBUVCXX_FS_POLL_H
