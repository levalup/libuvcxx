//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_TTY_H
#define LIBUVCXX_TTY_H

#include "stream.h"

namespace uv {
    class tty_t : public inherit_handle_t<uv_tty_t, stream_t> {
    public:
        using self = tty_t;
        using supper = inherit_handle_t<uv_tty_t, stream_t>;

        tty_t(uv_file fd, int unused) : self(default_loop(), fd, unused) {}

        explicit tty_t(const loop_t &loop, uv_file fd, int unused) {
            (void) uv_tty_init(loop, *this, fd, unused);
            // data will be deleted in close action
            set_data(new data_t(*this));
        }

        int fileno(uv_os_fd_t *fd) const {
            auto err = uv_fileno(*this, fd);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        stream_t accept(uv_file fd, int unused) {
            self client(this->loop(), fd, unused);
            uvcxx::defer close_client([&]() { client.close(nullptr); });

            auto err = uv_accept(*this, client);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);

            close_client.release();
            return client;
        }

        int set_mode(uv_tty_mode_t mode) {
            auto err = uv_tty_set_mode(*this, mode);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        int get_winsize(int *width, int *height) const {
            auto err = uv_tty_get_winsize(*this, width, height);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }
    };

    namespace tty {
        inline int reset_mode() {
            auto err = uv_tty_reset_mode();
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        inline void set_vterm_state(uv_tty_vtermstate_t state) {
            uv_tty_set_vterm_state(state);
        }

        inline int get_vterm_state(uv_tty_vtermstate_t *state) {
            auto err = uv_tty_get_vterm_state(state);
            if (err < 0) UVCXX_THROW_OR_RETURN(err, err);
            return err;
        }

        inline uv_tty_vtermstate_t get_vterm_state() {
            uv_tty_vtermstate_t state{};
            auto err = uv_tty_get_vterm_state(&state);
            if (err < 0) throw uvcxx::errcode(err);
            return state;
        }
    }
}

#endif //LIBUVCXX_TTY_H
