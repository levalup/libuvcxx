//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_TO_STRING_H
#define LIBUVCXX_TO_STRING_H

#include <string>
#include <sstream>
#include <tuple>
#include <initializer_list>

#include <uv.h>

namespace uvcxx {
    template<typename T, typename std::enable_if_t<
            std::is_convertible_v<std::string, decltype(std::to_string(std::declval<const T &>()))>,
            int> = 0>
    inline std::string to_string(const T &t) {
        return std::to_string(t);
    }

    inline std::string
    mask_string(int mask, const char *zero, const std::initializer_list<std::tuple<int, const char *>> &args) {
        if (!mask) return zero;
        std::ostringstream oss;
        bool sep = false;
        for (auto [v, s]: args) {
            if (mask & v) {
                if (sep) oss << "|";
                oss << s;
                sep = true;
            }
        }
        return oss.str();
    }

#pragma push_macro("UVCXX_CASE")
#pragma push_macro("UVCXX_DEFAULT")
#define UVCXX_CASE(v, s) case (v): return (s)
#define UVCXX_DEFAULT(s) default: return (s)

    inline const char *to_string(uv_run_mode t) {
        switch (t) {
            UVCXX_DEFAULT("unknown");
            UVCXX_CASE(UV_RUN_DEFAULT, "default");
            UVCXX_CASE(UV_RUN_ONCE, "once");
            UVCXX_CASE(UV_RUN_NOWAIT, "nowait");
        }
    }

    inline const char *to_string(uv_handle_type t) {
        switch (t) {
            UVCXX_DEFAULT("unknown");
            UVCXX_CASE(UV_ASYNC, "async");
            UVCXX_CASE(UV_CHECK, "check");
            UVCXX_CASE(UV_FS_EVENT, "fs_event");
            UVCXX_CASE(UV_FS_POLL, "fs_poll");
            UVCXX_CASE(UV_HANDLE, "handle");
            UVCXX_CASE(UV_IDLE, "idle");
            UVCXX_CASE(UV_NAMED_PIPE, "pipe");
            UVCXX_CASE(UV_POLL, "poll");
            UVCXX_CASE(UV_PREPARE, "prepare");
            UVCXX_CASE(UV_PROCESS, "process");
            UVCXX_CASE(UV_STREAM, "stream");
            UVCXX_CASE(UV_TCP, "tcp");
            UVCXX_CASE(UV_TIMER, "timer");
            UVCXX_CASE(UV_TTY, "tty");
            UVCXX_CASE(UV_UDP, "udp");
            UVCXX_CASE(UV_SIGNAL, "signal");
            UVCXX_CASE(UV_FILE, "file");
        }
    }

    inline const char *to_string(uv_req_type t) {
        switch (t) {
            UVCXX_DEFAULT("unknown");
            UVCXX_CASE(UV_REQ, "req");
            UVCXX_CASE(UV_CONNECT, "connect");
            UVCXX_CASE(UV_WRITE, "write");
            UVCXX_CASE(UV_SHUTDOWN, "shutdown");
            UVCXX_CASE(UV_UDP_SEND, "udp_send");
            UVCXX_CASE(UV_FS, "fs");
            UVCXX_CASE(UV_WORK, "work");
            UVCXX_CASE(UV_GETADDRINFO, "getaddrinfo");
            UVCXX_CASE(UV_GETNAMEINFO, "getnameinfo");
        }
    }

    inline std::string to_string(uv_poll_event t) {
        switch (t) {
            UVCXX_CASE(UV_READABLE, "readable");
            UVCXX_CASE(UV_WRITABLE, "writable");
            UVCXX_CASE(UV_DISCONNECT, "disconnect");
            UVCXX_CASE(UV_PRIORITIZED, "prioritized");
        }
        return mask_string(t, "notset", {
                {UV_READABLE,    "readable"},
                {UV_WRITABLE,    "writable"},
                {UV_DISCONNECT,  "disconnect"},
                {UV_PRIORITIZED, "prioritized"},
        });
    }

    inline std::string to_string(uv_process_flags t) {
        return mask_string(t, "notset", {
                {UV_PROCESS_SETUID,                     "setuid"},
                {UV_PROCESS_SETGID,                     "setgid"},
                {UV_PROCESS_WINDOWS_VERBATIM_ARGUMENTS, "verbatim-arguments"},
                {UV_PROCESS_DETACHED,                   "detached"},
                {UV_PROCESS_WINDOWS_HIDE,               "hide"},
                {UV_PROCESS_WINDOWS_HIDE_CONSOLE,       "hide-console"},
                {UV_PROCESS_WINDOWS_HIDE_GUI,           "hide-gui"},
        });
    }

    inline std::string to_string(uv_stdio_flags t) {
        return mask_string(t, "ignore", {
                {UV_CREATE_PIPE,    "create-pipe"},
                {UV_INHERIT_FD,     "inherit-fd"},
                {UV_INHERIT_STREAM, "inherit-stream"},
                {UV_READABLE_PIPE,  "readable-pipe"},
                {UV_WRITABLE_PIPE,  "writable-pipe"},
                {UV_NONBLOCK_PIPE,  "nonblock-pipe"},
        });
    }

    inline const char *to_string(uv_tty_mode_t t) {
        switch (t) {
            UVCXX_DEFAULT("unknown");
            UVCXX_CASE(UV_TTY_MODE_NORMAL, "normal");
            UVCXX_CASE(UV_TTY_MODE_RAW, "raw");
            UVCXX_CASE(UV_TTY_MODE_IO, "io");
        }
    }

    inline const char *to_string(uv_tty_vtermstate_t t) {
        switch (t) {
            UVCXX_DEFAULT("unknown");
            UVCXX_CASE(UV_TTY_SUPPORTED, "supported");
            UVCXX_CASE(UV_TTY_UNSUPPORTED, "unsupported");
        }
    }

    inline std::string to_string(uv_udp_flags t) {
        return mask_string(t, "notset", {
                {UV_UDP_IPV6ONLY,      "ipv6only"},
                {UV_UDP_PARTIAL,       "partial"},
                {UV_UDP_REUSEADDR,     "reuseaddr"},
                {UV_UDP_MMSG_CHUNK,    "mmsg-chunk"},
                {UV_UDP_MMSG_FREE,     "mmsg-free"},
                {UV_UDP_LINUX_RECVERR, "linux-recverr"},
                {UV_UDP_RECVMMSG,      "recvmmsg"},
        });
    }

    inline const char *to_string(uv_membership t) {
        switch (t) {
            UVCXX_DEFAULT("unknown");
            UVCXX_CASE(UV_LEAVE_GROUP, "leave-group");
            UVCXX_CASE(UV_JOIN_GROUP, "join-group");
        }
    }

    inline const char *to_string(uv_fs_event t) {
        switch (t) {
            UVCXX_DEFAULT("unknown");
            UVCXX_CASE(UV_RENAME, "rename");
            UVCXX_CASE(UV_CHANGE, "change");
        }
    }

    inline std::string to_string(uv_fs_event_flags t) {
        return mask_string(t, "notset", {
                {UV_FS_EVENT_WATCH_ENTRY, "watch-entry"},
                {UV_FS_EVENT_STAT,        "stat"},
                {UV_FS_EVENT_RECURSIVE,   "recursive"},
        });
    }

    inline const char *to_string(uv_fs_type t) {
        switch (t) {
            UVCXX_DEFAULT("unknown");
            UVCXX_CASE(UV_FS_CUSTOM, "custom");
            UVCXX_CASE(UV_FS_OPEN, "open");
            UVCXX_CASE(UV_FS_CLOSE, "close");
            UVCXX_CASE(UV_FS_READ, "read");
            UVCXX_CASE(UV_FS_WRITE, "write");
            UVCXX_CASE(UV_FS_SENDFILE, "sendfile");
            UVCXX_CASE(UV_FS_STAT, "stat");
            UVCXX_CASE(UV_FS_LSTAT, "lstat");
            UVCXX_CASE(UV_FS_FSTAT, "fstat");
            UVCXX_CASE(UV_FS_FTRUNCATE, "ftruncate");
            UVCXX_CASE(UV_FS_UTIME, "utime");
            UVCXX_CASE(UV_FS_FUTIME, "futime");
            UVCXX_CASE(UV_FS_ACCESS, "access");
            UVCXX_CASE(UV_FS_CHMOD, "chomd");
            UVCXX_CASE(UV_FS_FCHMOD, "fchmod");
            UVCXX_CASE(UV_FS_FSYNC, "fsync");
            UVCXX_CASE(UV_FS_FDATASYNC, "fdatasync");
            UVCXX_CASE(UV_FS_UNLINK, "unlink");
            UVCXX_CASE(UV_FS_RMDIR, "rmdir");
            UVCXX_CASE(UV_FS_MKDIR, "mkdir");
            UVCXX_CASE(UV_FS_MKDTEMP, "mkdtemp");
            UVCXX_CASE(UV_FS_RENAME, "rename");
            UVCXX_CASE(UV_FS_SCANDIR, "scandir");
            UVCXX_CASE(UV_FS_LINK, "link");
            UVCXX_CASE(UV_FS_SYMLINK, "symlink");
            UVCXX_CASE(UV_FS_READLINK, "readlink");
            UVCXX_CASE(UV_FS_CHOWN, "chown");
            UVCXX_CASE(UV_FS_FCHOWN, "fchown");
            UVCXX_CASE(UV_FS_REALPATH, "realpath");
            UVCXX_CASE(UV_FS_COPYFILE, "copyfile");
            UVCXX_CASE(UV_FS_LCHOWN, "lchown");
            UVCXX_CASE(UV_FS_OPENDIR, "opendir");
            UVCXX_CASE(UV_FS_READDIR, "readdir");
            UVCXX_CASE(UV_FS_CLOSEDIR, "closedir");
            UVCXX_CASE(UV_FS_MKSTEMP, "mkstemp");
            UVCXX_CASE(UV_FS_LUTIME, "lutime");
        }
    }

    inline const char *to_string(uv_dirent_type_t t) {
        switch (t) {
            UVCXX_DEFAULT("unknown");
            UVCXX_CASE(UV_DIRENT_FILE, "file");
            UVCXX_CASE(UV_DIRENT_DIR, "dir");
            UVCXX_CASE(UV_DIRENT_LINK, "link");
            UVCXX_CASE(UV_DIRENT_FIFO, "fifo");
            UVCXX_CASE(UV_DIRENT_SOCKET, "socket");
            UVCXX_CASE(UV_DIRENT_CHAR, "char");
            UVCXX_CASE(UV_DIRENT_BLOCK, "block");
        }
    }

    inline const char *to_string(uv_clock_id t) {
        switch (t) {
            UVCXX_DEFAULT("unknown");
            UVCXX_CASE(UV_CLOCK_MONOTONIC, "monotonic");
            UVCXX_CASE(UV_CLOCK_REALTIME, "realtime");
        }
    }
}

#pragma pop_macro("UVCXX_CASE")
#pragma pop_macro("UVCXX_DEFAULT")


#endif //LIBUVCXX_TO_STRING_H
