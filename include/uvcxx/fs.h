//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_FS_H
#define LIBUVCXX_FS_H

#include <vector>

#include "buf.h"
#include "loop.h"
#include "req.h"

namespace uv {
    class fs_t : public inherit_req_t<uv_fs_t, req_t> {
    public:
        using self = fs_t;
        using supper = inherit_req_t<uv_fs_t, req_t>;

        [[nodiscard]]
        loop_t loop() const {
            return loop_t::borrow(raw<raw_t>()->loop);
        }

        [[nodiscard]]
        uv_fs_type fs_type() const {
            return raw<raw_t>()->fs_type;
        }

        [[nodiscard]]
        const char *path() const {
            return raw<raw_t>()->path;
        }

        [[nodiscard]]
        ssize_t result() const {
            return raw<raw_t>()->result;
        }

        [[nodiscard]]
        uv_stat_t statbuf() const {
            return raw<raw_t>()->statbuf;
        }

        [[nodiscard]]
        void *ptr() const {
            return raw<raw_t>()->ptr;
        }

        [[nodiscard]]
        uv_fs_type get_fs_type() const {
            return raw<raw_t>()->fs_type;
        }

        [[nodiscard]]
        ssize_t get_result() const {
            return raw<raw_t>()->result;
        }

#if UVCXX_SATISFY_VERSION(1, 38, 0)

        [[nodiscard]]
        int get_system_error() const {
            return uv_fs_get_system_error(*this);
        }

#endif

        [[nodiscard]]
        void *get_ptr() const {
            return raw<raw_t>()->ptr;
        }

        [[nodiscard]]
        const char *get_path() const {
            return raw<raw_t>()->path;
        }

        uv_stat_t *get_statbuf() {
            return &raw<raw_t>()->statbuf;
        }

        template<typename T>
        [[nodiscard]]
        T *ptr() const {
            return (T *) raw<raw_t>()->ptr;
        }
    };

    namespace fs {
        namespace inner {
            using cxx_req_t = fs_t;
            using raw_req_t = cxx_req_t::raw_t;

            template<typename... T>
            class callback_t : public req_callback_t<raw_req_t> {
            public:
                using self = callback_t;
                using supper = req_callback_t<raw_req_t>;

                using promise_t = uvcxx::promise<T...>;
                using promise_cast_t = uvcxx::promise_cast<uvcxx::promise<T...>, raw_req_t *>;

                promise_cast_t promise;

                explicit callback_t(const cxx_req_t &req, typename promise_cast_t::wrapper_t wrapper)
                        : supper(req), promise(std::move(wrapper)) {
                }

                uvcxx::promise_proxy<raw_req_t *> &proxy() noexcept final {
                    return promise;
                }

                void finalize(raw_req_t *req) noexcept final {
                    uv_fs_req_cleanup(req);
                }

                int check(raw_req_t *req) noexcept final {
                    return (int) req->result;
                }
            };

            template<typename... T>
            class invoker {
            public:
                template<typename FUNC, typename ...ARGS,
                        typename=typename std::enable_if_t<std::is_integral_v<
                                std::invoke_result_t<FUNC,
                                        loop_t::raw_t *, raw_req_t *, ARGS..., decltype(callback_t<T...>::raw_callback)>
                        >>>
                uvcxx::promise<T...> operator()(
                        typename callback_t<T...>::promise_cast_t::wrapper_t wrapper,
                        FUNC func, const loop_t &loop, const cxx_req_t &req, ARGS &&...args) const {
                    auto *data = new callback_t<T...>(req, std::move(wrapper));
                    uvcxx::defer_delete delete_data(data);

                    auto err = std::invoke(
                            func,
                            (loop_t::raw_t *) loop,
                            (cxx_req_t::raw_t *) data->req(),
                            std::forward<ARGS>(args)...,
                            callback_t<T...>::raw_callback);

                    if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);
                    delete_data.release();
                    return data->promise.promise();
                }
            };
        }

        [[nodiscard]]
        inline uvcxx::promise<uv_file> open(
                const loop_t &loop, const fs_t &fs, const char *path, int flags, int mode = 0644) {
            return inner::invoker<uv_file>()(
                    [](inner::raw_req_t *req) { return (uv_file) req->result; },
                    uv_fs_open, loop, fs, path, flags, mode);
        }

        [[nodiscard]]
        inline uvcxx::promise<uv_file> open(const char *path, int flags, int mode = 0644) {
            return open(default_loop(), {}, path, flags, mode);
        }

        [[nodiscard]]
        inline uvcxx::promise<uv_file> open(const loop_t &loop, const char *path, int flags, int mode = 0644) {
            return open(loop, {}, path, flags, mode);
        }

        [[nodiscard]]
        inline uvcxx::promise<uv_file> open(const fs_t &fs, const char *path, int flags, int mode = 0644) {
            return open(default_loop(), fs, path, flags, mode);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> close(const loop_t &loop, const fs_t &fs, uv_file file) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_close, loop, fs, file);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> close(uv_file file) {
            return close(default_loop(), {}, file);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> close(const loop_t &loop, uv_file file) {
            return close(loop, {}, file);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> close(const fs_t &fs, uv_file file) {
            return close(default_loop(), fs, file);
        }

        [[nodiscard]]
        inline uvcxx::promise<ssize_t> read(
                const loop_t &loop, const fs_t &fs,
                uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset = -1) {
            return inner::invoker<ssize_t>()(
                    [](inner::raw_req_t *req) { return (ssize_t) req->result; },
                    uv_fs_read, loop, fs, file, bufs, nbufs, offset);
        }

        [[nodiscard]]
        inline uvcxx::promise<ssize_t> read(
                const loop_t &loop, const fs_t &fs,
                uv_file file, buf_t &buf, int64_t offset = -1) {
            return read(loop, fs, file, buf, 1, offset);
        }

        [[nodiscard]]
        inline uvcxx::promise<ssize_t> read(
                const loop_t &loop, const fs_t &fs,
                uv_file file, std::vector<buf_t> &bufs, int64_t offset = -1) {
            std::vector<buf_t::raw_t> raw_bufs(bufs.size());
            for (size_t i = 0; i < bufs.size(); ++i) {
                raw_bufs[i] = *(buf_t::raw_t *) bufs[i];
            }
            auto nbufs = (unsigned int) bufs.size();
            return read(loop, fs, file, raw_bufs.data(), nbufs, offset);
        }

        [[nodiscard]]
        inline uvcxx::promise<ssize_t> read(
                uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset = -1) {
            return read(default_loop(), {}, file, bufs, nbufs, offset);
        }

        [[nodiscard]]
        inline uvcxx::promise<ssize_t> read(
                const loop_t &loop,
                uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset = -1) {
            return read(loop, {}, file, bufs, nbufs, offset);
        }

        [[nodiscard]]
        inline uvcxx::promise<ssize_t> read(
                const fs_t &fs,
                uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset = -1) {
            return read(default_loop(), fs, file, bufs, nbufs, offset);
        }

        [[nodiscard]]
        inline uvcxx::promise<ssize_t> read(
                uv_file file, buf_t &buf, int64_t offset = -1) {
            return read(default_loop(), {}, file, buf, offset);
        }

        [[nodiscard]]
        inline uvcxx::promise<ssize_t> read(
                const loop_t &loop,
                uv_file file, buf_t &buf, int64_t offset = -1) {
            return read(loop, {}, file, buf, offset);
        }

        [[nodiscard]]
        inline uvcxx::promise<ssize_t> read(
                const fs_t &fs,
                uv_file file, buf_t &buf, int64_t offset = -1) {
            return read(default_loop(), fs, file, buf, offset);
        }

        [[nodiscard]]
        inline uvcxx::promise<ssize_t> read(
                uv_file file, std::vector<buf_t> &bufs, int64_t offset = -1) {
            return read(default_loop(), {}, file, bufs, offset);
        }

        [[nodiscard]]
        inline uvcxx::promise<ssize_t> read(
                const loop_t &loop,
                uv_file file, std::vector<buf_t> &bufs, int64_t offset = -1) {
            return read(loop, {}, file, bufs, offset);
        }

        [[nodiscard]]
        inline uvcxx::promise<ssize_t> read(
                const fs_t &fs,
                uv_file file, std::vector<buf_t> &bufs, int64_t offset = -1) {
            return read(default_loop(), fs, file, bufs, offset);
        }

        [[nodiscard]]
        inline uvcxx::promise<ssize_t> write(
                const loop_t &loop, const fs_t &fs,
                uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset = -1) {
            return inner::invoker<ssize_t>()(
                    [](inner::raw_req_t *req) { return (ssize_t) req->result; },
                    uv_fs_write, loop, fs, file, bufs, nbufs, offset);
        }

        [[nodiscard]]
        inline uvcxx::promise<ssize_t> write(
                const loop_t &loop, const fs_t &fs,
                uv_file file, buf_t &buf, int64_t offset = -1) {
            return write(loop, fs, file, buf, 1, offset);
        }

        [[nodiscard]]
        inline uvcxx::promise<ssize_t> write(
                const loop_t &loop, const fs_t &fs,
                uv_file file, std::vector<buf_t> &bufs, int64_t offset = -1) {
            std::vector<buf_t::raw_t> raw_bufs(bufs.size());
            for (size_t i = 0; i < bufs.size(); ++i) {
                raw_bufs[i] = *(buf_t::raw_t *) bufs[i];
            }
            auto nbufs = (unsigned int) bufs.size();
            return write(loop, fs, file, raw_bufs.data(), nbufs, offset);
        }

        [[nodiscard]]
        inline uvcxx::promise<ssize_t> write(
                uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset = -1) {
            return write(default_loop(), {}, file, bufs, nbufs, offset);
        }

        [[nodiscard]]
        inline uvcxx::promise<ssize_t> write(
                const loop_t &loop,
                uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset = -1) {
            return write(loop, {}, file, bufs, nbufs, offset);
        }

        [[nodiscard]]
        inline uvcxx::promise<ssize_t> write(
                const fs_t &fs,
                uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset = -1) {
            return write(default_loop(), fs, file, bufs, nbufs, offset);
        }

        [[nodiscard]]
        inline uvcxx::promise<ssize_t> write(
                uv_file file, buf_t &buf, int64_t offset = -1) {
            return write(default_loop(), {}, file, buf, offset);
        }

        [[nodiscard]]
        inline uvcxx::promise<ssize_t> write(
                const loop_t &loop,
                uv_file file, buf_t &buf, int64_t offset = -1) {
            return write(loop, {}, file, buf, offset);
        }

        [[nodiscard]]
        inline uvcxx::promise<ssize_t> write(
                const fs_t &fs,
                uv_file file, buf_t &buf, int64_t offset = -1) {
            return write(default_loop(), fs, file, buf, offset);
        }

        [[nodiscard]]
        inline uvcxx::promise<ssize_t> write(
                uv_file file, std::vector<buf_t> &bufs, int64_t offset = -1) {
            return write(default_loop(), {}, file, bufs, offset);
        }

        [[nodiscard]]
        inline uvcxx::promise<ssize_t> write(
                const loop_t &loop,
                uv_file file, std::vector<buf_t> &bufs, int64_t offset = -1) {
            return write(loop, {}, file, bufs, offset);
        }

        [[nodiscard]]
        inline uvcxx::promise<ssize_t> write(
                const fs_t &fs,
                uv_file file, std::vector<buf_t> &bufs, int64_t offset = -1) {
            return write(default_loop(), fs, file, bufs, offset);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> unlink(const loop_t &loop, const fs_t &fs, const char *path) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_unlink, loop, fs, path);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> unlink(const char *path) {
            return unlink(default_loop(), {}, path);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> unlink(const loop_t &loop, const char *path) {
            return unlink(loop, {}, path);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> unlink(const fs_t &fs, const char *path) {
            return unlink(default_loop(), fs, path);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> mkdir(const loop_t &loop, const fs_t &fs, const char *path, int mode = 0755) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_mkdir, loop, fs, path, mode);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> mkdir(const char *path, int mode = 0755) {
            return mkdir(default_loop(), {}, path, mode);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> mkdir(const loop_t &loop, const char *path, int mode = 0755) {
            return mkdir(loop, {}, path, mode);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> mkdir(const fs_t &fs, const char *path, int mode = 0755) {
            return mkdir(default_loop(), fs, path, mode);
        }

        [[nodiscard]]
        inline uvcxx::promise<const char *> mkdtemp(const loop_t &loop, const fs_t &fs, const char *tpl) {
            return inner::invoker<const char *>()(
                    [](inner::raw_req_t *req) { return req->path; },
                    uv_fs_mkdtemp, loop, fs, tpl);
        }

        [[nodiscard]]
        inline uvcxx::promise<const char *> mkdtemp(const char *tpl) {
            return mkdtemp(default_loop(), {}, tpl);
        }

        [[nodiscard]]
        inline uvcxx::promise<const char *> mkdtemp(const loop_t &loop, const char *tpl) {
            return mkdtemp(loop, {}, tpl);
        }

        [[nodiscard]]
        inline uvcxx::promise<const char *> mkdtemp(const fs_t &fs, const char *tpl) {
            return mkdtemp(default_loop(), fs, tpl);
        }

#if UVCXX_SATISFY_VERSION(1, 34, 0)

        [[nodiscard]]
        inline uvcxx::promise<int, const char *> mkstemp(const loop_t &loop, const fs_t &fs, const char *tpl) {
            return inner::invoker<int, const char *>()(
                    [](inner::raw_req_t *req) { return std::make_tuple((int) req->result, req->path); },
                    uv_fs_mkstemp, loop, fs, tpl);
        }

        [[nodiscard]]
        inline uvcxx::promise<int, const char *> mkstemp(const char *tpl) {
            return mkstemp(default_loop(), {}, tpl);
        }

        [[nodiscard]]
        inline uvcxx::promise<int, const char *> mkstemp(const loop_t &loop, const char *tpl) {
            return mkstemp(loop, {}, tpl);
        }

        [[nodiscard]]
        inline uvcxx::promise<int, const char *> mkstemp(const fs_t &fs, const char *tpl) {
            return mkstemp(default_loop(), fs, tpl);
        }

#endif

        [[nodiscard]]
        inline uvcxx::promise<int> rmdir(const loop_t &loop, const fs_t &fs, const char *path) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_rmdir, loop, fs, path);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> rmdir(const char *path) {
            return rmdir(default_loop(), {}, path);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> rmdir(const loop_t &loop, const char *path) {
            return rmdir(loop, {}, path);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> rmdir(const fs_t &fs, const char *path) {
            return rmdir(default_loop(), fs, path);
        }

#if UVCXX_SATISFY_VERSION(1, 28, 0)

        [[nodiscard]]
        inline uvcxx::promise<uv_dir_t *> opendir(const loop_t &loop, const fs_t &fs, const char *path) {
            return inner::invoker<uv_dir_t *>()(
                    [](inner::raw_req_t *req) { return (uv_dir_t *) req->ptr; },
                    uv_fs_opendir, loop, fs, path);
        }

        [[nodiscard]]
        inline uvcxx::promise<uv_dir_t *> opendir(const char *path) {
            return opendir(default_loop(), {}, path);
        }

        [[nodiscard]]
        inline uvcxx::promise<uv_dir_t *> opendir(const loop_t &loop, const char *path) {
            return opendir(loop, {}, path);
        }

        [[nodiscard]]
        inline uvcxx::promise<uv_dir_t *> opendir(const fs_t &fs, const char *path) {
            return opendir(default_loop(), fs, path);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> closedir(const loop_t &loop, const fs_t &fs, uv_dir_t *dir) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_closedir, loop, fs, dir);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> closedir(uv_dir_t *dir) {
            return closedir(default_loop(), {}, dir);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> closedir(const loop_t &loop, uv_dir_t *dir) {
            return closedir(loop, {}, dir);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> closedir(const fs_t &fs, uv_dir_t *dir) {
            return closedir(default_loop(), fs, dir);
        }

#endif

        [[nodiscard]]
        inline uvcxx::promise<uv_stat_t *> stat(const loop_t &loop, const fs_t &fs, const char *path) {
            return inner::invoker<uv_stat_t *>()(
                    [](inner::raw_req_t *req) { return &req->statbuf; },
                    uv_fs_stat, loop, fs, path);
        }

        [[nodiscard]]
        inline uvcxx::promise<uv_stat_t *> stat(const char *path) {
            return stat(default_loop(), {}, path);
        }

        [[nodiscard]]
        inline uvcxx::promise<uv_stat_t *> stat(const loop_t &loop, const char *path) {
            return stat(loop, {}, path);
        }

        [[nodiscard]]
        inline uvcxx::promise<uv_stat_t *> stat(const fs_t &fs, const char *path) {
            return stat(default_loop(), fs, path);
        }

        [[nodiscard]]
        inline uvcxx::promise<uv_stat_t *> fstat(const loop_t &loop, const fs_t &fs, uv_file file) {
            return inner::invoker<uv_stat_t *>()(
                    [](inner::raw_req_t *req) { return &req->statbuf; },
                    uv_fs_fstat, loop, fs, file);
        }

        [[nodiscard]]
        inline uvcxx::promise<uv_stat_t *> fstat(uv_file file) {
            return fstat(default_loop(), {}, file);
        }

        [[nodiscard]]
        inline uvcxx::promise<uv_stat_t *> fstat(const loop_t &loop, uv_file file) {
            return fstat(loop, {}, file);
        }

        [[nodiscard]]
        inline uvcxx::promise<uv_stat_t *> fstat(const fs_t &fs, uv_file file) {
            return fstat(default_loop(), fs, file);
        }

        [[nodiscard]]
        inline uvcxx::promise<uv_stat_t *> lstat(const loop_t &loop, const fs_t &fs, const char *path) {
            return inner::invoker<uv_stat_t *>()(
                    [](inner::raw_req_t *req) { return &req->statbuf; },
                    uv_fs_lstat, loop, fs, path);
        }

        [[nodiscard]]
        inline uvcxx::promise<uv_stat_t *> lstat(const char *path) {
            return lstat(default_loop(), {}, path);
        }

        [[nodiscard]]
        inline uvcxx::promise<uv_stat_t *> lstat(const loop_t &loop, const char *path) {
            return lstat(loop, {}, path);
        }

        [[nodiscard]]
        inline uvcxx::promise<uv_stat_t *> lstat(const fs_t &fs, const char *path) {
            return lstat(default_loop(), fs, path);
        }

#if UVCXX_SATISFY_VERSION(1, 31, 0)

        [[nodiscard]]
        inline uvcxx::promise<uv_statfs_t *> statfs(const loop_t &loop, const fs_t &fs, const char *path) {
            return inner::invoker<uv_statfs_t *>()(
                    [](inner::raw_req_t *req) { return (uv_statfs_t *) req->ptr; },
                    uv_fs_statfs, loop, fs, path);
        }

        [[nodiscard]]
        inline uvcxx::promise<uv_statfs_t *> statfs(const char *path) {
            return statfs(default_loop(), {}, path);
        }

        [[nodiscard]]
        inline uvcxx::promise<uv_statfs_t *> statfs(const loop_t &loop, const char *path) {
            return statfs(loop, {}, path);
        }

        [[nodiscard]]
        inline uvcxx::promise<uv_statfs_t *> statfs(const fs_t &fs, const char *path) {
            return statfs(default_loop(), fs, path);
        }

#endif

        [[nodiscard]]
        inline uvcxx::promise<int> rename(
                const loop_t &loop, const fs_t &fs, const char *path, const char *new_path) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_rename, loop, fs, path, new_path);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> rename(const char *path, const char *new_path) {
            return rename(default_loop(), {}, path, new_path);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> rename(const loop_t &loop, const char *path, const char *new_path) {
            return rename(loop, {}, path, new_path);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> rename(const fs_t &fs, const char *path, const char *new_path) {
            return rename(default_loop(), fs, path, new_path);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> fsync(const loop_t &loop, const fs_t &fs, uv_file file) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_fsync, loop, fs, file);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> fsync(uv_file file) {
            return fsync(default_loop(), {}, file);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> fsync(const loop_t &loop, uv_file file) {
            return fsync(loop, {}, file);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> fsync(const fs_t &fs, uv_file file) {
            return fsync(default_loop(), fs, file);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> fdatasync(const loop_t &loop, const fs_t &fs, uv_file file) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_fdatasync, loop, fs, file);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> fdatasync(uv_file file) {
            return fdatasync(default_loop(), {}, file);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> fdatasync(const loop_t &loop, uv_file file) {
            return fdatasync(loop, {}, file);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> fdatasync(const fs_t &fs, uv_file file) {
            return fdatasync(default_loop(), fs, file);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> ftruncate(
                const loop_t &loop, const fs_t &fs, uv_file file, int64_t offset) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_ftruncate, loop, fs, file, offset);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> ftruncate(uv_file file, int64_t offset) {
            return ftruncate(default_loop(), {}, file, offset);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> ftruncate(const loop_t &loop, uv_file file, int64_t offset) {
            return ftruncate(loop, {}, file, offset);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> ftruncate(const fs_t &fs, uv_file file, int64_t offset) {
            return ftruncate(default_loop(), fs, file, offset);
        }

#if UVCXX_SATISFY_VERSION(1, 14, 0)

        [[nodiscard]]
        inline uvcxx::promise<int> copyfile(
                const loop_t &loop, const fs_t &fs, const char *path, const char *new_path, int flags) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_copyfile, loop, fs, path, new_path, flags);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> copyfile(const char *path, const char *new_path, int flags) {
            return copyfile(default_loop(), {}, path, new_path, flags);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> copyfile(const loop_t &loop, const char *path, const char *new_path, int flags) {
            return copyfile(loop, {}, path, new_path, flags);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> copyfile(const fs_t &fs, const char *path, const char *new_path, int flags) {
            return copyfile(default_loop(), fs, path, new_path, flags);
        }

#endif

        [[nodiscard]]
        inline uvcxx::promise<ssize_t> sendfile(
                const loop_t &loop, const fs_t &fs,
                uv_file out_fd, uv_file in_fd, int64_t in_offset, size_t length) {
            return inner::invoker<ssize_t>()(
                    [](inner::raw_req_t *req) { return (ssize_t) req->result; },
                    uv_fs_sendfile, loop, fs, out_fd, in_fd, in_offset, length);
        }

        [[nodiscard]]
        inline uvcxx::promise<ssize_t> sendfile(
                uv_file out_fd, uv_file in_fd, int64_t in_offset, size_t length) {
            return sendfile(default_loop(), {}, out_fd, in_fd, in_offset, length);
        }

        [[nodiscard]]
        inline uvcxx::promise<ssize_t> sendfile(
                const loop_t &loop, uv_file out_fd, uv_file in_fd, int64_t in_offset, size_t length) {
            return sendfile(loop, {}, out_fd, in_fd, in_offset, length);
        }

        [[nodiscard]]
        inline uvcxx::promise<ssize_t> sendfile(
                const fs_t &fs, uv_file out_fd, uv_file in_fd, int64_t in_offset, size_t length) {
            return sendfile(default_loop(), fs, out_fd, in_fd, in_offset, length);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> access(const loop_t &loop, const fs_t &fs, const char *path, int mode) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_access, loop, fs, path, mode);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> access(const char *path, int mode) {
            return access(default_loop(), {}, path, mode);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> access(const loop_t &loop, const char *path, int mode) {
            return access(loop, {}, path, mode);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> access(const fs_t &fs, const char *path, int mode) {
            return access(default_loop(), fs, path, mode);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> chmod(const loop_t &loop, const fs_t &fs, const char *path, int mode) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_chmod, loop, fs, path, mode);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> chmod(const char *path, int mode) {
            return chmod(default_loop(), {}, path, mode);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> chmod(const loop_t &loop, const char *path, int mode) {
            return chmod(loop, {}, path, mode);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> chmod(const fs_t &fs, const char *path, int mode) {
            return chmod(default_loop(), fs, path, mode);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> fchmod(const loop_t &loop, const fs_t &fs, uv_file file, int mode) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_fchmod, loop, fs, file, mode);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> fchmod(uv_file file, int mode) {
            return fchmod(default_loop(), {}, file, mode);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> fchmod(const loop_t &loop, uv_file file, int mode) {
            return fchmod(loop, {}, file, mode);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> fchmod(const fs_t &fs, uv_file file, int mode) {
            return fchmod(default_loop(), fs, file, mode);
        }

        [[nodiscard]]
        inline uvcxx::promise<int>
        utime(const loop_t &loop, const fs_t &fs, const char *path, double atime, double mtime) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_utime, loop, fs, path, atime, mtime);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> utime(const char *path, double atime, double mtime) {
            return utime(default_loop(), {}, path, atime, mtime);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> utime(const loop_t &loop, const char *path, double atime, double mtime) {
            return utime(loop, {}, path, atime, mtime);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> utime(const fs_t &fs, const char *path, double atime, double mtime) {
            return utime(default_loop(), fs, path, atime, mtime);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> futime(
                const loop_t &loop, const fs_t &fs, uv_file file, double atime, double mtime) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_futime, loop, fs, file, atime, mtime);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> futime(uv_file file, double atime, double mtime) {
            return futime(default_loop(), {}, file, atime, mtime);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> futime(const loop_t &loop, uv_file file, double atime, double mtime) {
            return futime(loop, {}, file, atime, mtime);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> futime(const fs_t &fs, uv_file file, double atime, double mtime) {
            return futime(default_loop(), fs, file, atime, mtime);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> lutime(
                const loop_t &loop, const fs_t &fs, const char *path, double atime, double mtime) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_lutime, loop, fs, path, atime, mtime);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> lutime(const char *path, double atime, double mtime) {
            return lutime(default_loop(), {}, path, atime, mtime);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> lutime(const loop_t &loop, const char *path, double atime, double mtime) {
            return lutime(loop, {}, path, atime, mtime);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> lutime(const fs_t &fs, const char *path, double atime, double mtime) {
            return lutime(default_loop(), fs, path, atime, mtime);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> link(
                const loop_t &loop, const fs_t &fs, const char *path, const char *new_path) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_link, loop, fs, path, new_path);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> link(const char *path, const char *new_path) {
            return link(default_loop(), {}, path, new_path);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> link(const loop_t &loop, const char *path, const char *new_path) {
            return link(loop, {}, path, new_path);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> link(const fs_t &fs, const char *path, const char *new_path) {
            return link(default_loop(), fs, path, new_path);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> symlink(
                const loop_t &loop, const fs_t &fs, const char *path, const char *new_path, int flags) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_symlink, loop, fs, path, new_path, flags);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> symlink(const char *path, const char *new_path, int flags) {
            return symlink(default_loop(), {}, path, new_path, flags);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> symlink(const loop_t &loop, const char *path, const char *new_path, int flags) {
            return symlink(loop, {}, path, new_path, flags);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> symlink(const fs_t &fs, const char *path, const char *new_path, int flags) {
            return symlink(default_loop(), fs, path, new_path, flags);
        }

        [[nodiscard]]
        inline uvcxx::promise<const char *> readlink(const loop_t &loop, const fs_t &fs, const char *path) {
            return inner::invoker<const char *>()(
                    [](inner::raw_req_t *req) { return (const char *) req->ptr; },
                    uv_fs_readlink, loop, fs, path);
        }

        [[nodiscard]]
        inline uvcxx::promise<const char *> readlink(const char *path) {
            return readlink(default_loop(), {}, path);
        }

        [[nodiscard]]
        inline uvcxx::promise<const char *> readlink(const loop_t &loop, const char *path) {
            return readlink(loop, {}, path);
        }

        [[nodiscard]]
        inline uvcxx::promise<const char *> readlink(const fs_t &fs, const char *path) {
            return readlink(default_loop(), fs, path);
        }

#if UVCXX_SATISFY_VERSION(1, 8, 0)

        [[nodiscard]]
        inline uvcxx::promise<const char *> realpath(const loop_t &loop, const fs_t &fs, const char *path) {
            return inner::invoker<const char *>()(
                    [](inner::raw_req_t *req) { return (const char *) req->ptr; },
                    uv_fs_realpath, loop, fs, path);
        }

        [[nodiscard]]
        inline uvcxx::promise<const char *> realpath(const char *path) {
            return realpath(default_loop(), {}, path);
        }

        [[nodiscard]]
        inline uvcxx::promise<const char *> realpath(const loop_t &loop, const char *path) {
            return realpath(loop, {}, path);
        }

        [[nodiscard]]
        inline uvcxx::promise<const char *> realpath(const fs_t &fs, const char *path) {
            return realpath(default_loop(), fs, path);
        }

#endif

        [[nodiscard]]
        inline uvcxx::promise<int> chown(
                const loop_t &loop, const fs_t &fs, const char *path, uv_uid_t uid, uv_gid_t gid) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_chown, loop, fs, path, uid, gid);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> chown(const char *path, uv_uid_t uid, uv_gid_t gid) {
            return chown(default_loop(), {}, path, uid, gid);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> chown(const loop_t &loop, const char *path, uv_uid_t uid, uv_gid_t gid) {
            return chown(loop, {}, path, uid, gid);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> chown(const fs_t &fs, const char *path, uv_uid_t uid, uv_gid_t gid) {
            return chown(default_loop(), fs, path, uid, gid);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> fchown(
                const loop_t &loop, const fs_t &fs, uv_file file, uv_uid_t uid, uv_gid_t gid) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_fchown, loop, fs, file, uid, gid);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> fchown(uv_file file, uv_uid_t uid, uv_gid_t gid) {
            return fchown(default_loop(), {}, file, uid, gid);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> fchown(const loop_t &loop, uv_file file, uv_uid_t uid, uv_gid_t gid) {
            return fchown(loop, {}, file, uid, gid);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> fchown(const fs_t &fs, uv_file file, uv_uid_t uid, uv_gid_t gid) {
            return fchown(default_loop(), fs, file, uid, gid);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> lchown(
                const loop_t &loop, const fs_t &fs, const char *path, uv_uid_t uid, uv_gid_t gid) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_lchown, loop, fs, path, uid, gid);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> lchown(const char *path, uv_uid_t uid, uv_gid_t gid) {
            return lchown(default_loop(), {}, path, uid, gid);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> lchown(const loop_t &loop, const char *path, uv_uid_t uid, uv_gid_t gid) {
            return lchown(loop, {}, path, uid, gid);
        }

        [[nodiscard]]
        inline uvcxx::promise<int> lchown(const fs_t &fs, const char *path, uv_uid_t uid, uv_gid_t gid) {
            return lchown(default_loop(), fs, path, uid, gid);
        }
    }
}


#endif //LIBUVCXX_FS_H
