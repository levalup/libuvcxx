//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_FS_EVENT_H
#define LIBUVCXX_FS_EVENT_H

#include "handle.h"
#include "platform.h"

namespace uv {
    class fs_event_t : public inherit_handle_t<uv_fs_event_t, handle_t> {
    public:
        using self = fs_event_t;
        using supper = inherit_handle_t<uv_fs_event_t, handle_t>;

        class data_t : supper::data_t {
        public:
            using self = data_t;
            using supper = supper::data_t;

            uvcxx::callback_emitter<const char *, int, int> start_cb;

            explicit data_t(fs_event_t &handle)
                    : supper(handle) {
                handle.watch(start_cb);
            }

            void close() noexcept final {
                // finally at close, make queue safe
                start_cb.finalize();
            }
        };

        fs_event_t() : self(default_loop()) {}

        explicit fs_event_t(const loop_t &loop) {
            (void) uv_fs_event_init(loop, *this);
            // data will be deleted in close action
            set_data(new data_t(*this));
        }

        [[nodiscard]]
        uvcxx::callback<const char *, int, int> start(const char *path, int flags) {
#if UVCXX_OS_MAC || UVCXX_OS_WINDOWS
            auto err = uv_fs_event_start(*this, raw_callback, path, flags);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);
            auto data = get_data<data_t>();
            return data->start_cb.callback();
#else
            throw uvcxx::errcode(UV_EPERM);
#endif
        }

        void stop() {
            (void) uv_fs_event_stop(*this);
        }

        int getpath(char *buffer, size_t *size) {
            return uv_fs_event_getpath(*this, buffer, size);
        }

    private:
        static void raw_callback(raw_t *handle, const char *filename, int events, int status) {
            auto data = (data_t *) (handle->data);
            data->start_cb.emit(filename, events, status);
        }
    };
}

#endif //LIBUVCXX_FS_EVENT_H
