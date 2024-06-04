//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_FS_H
#define LIBUVCXX_FS_H

#include <functional>
#include <utility>
#include <vector>

#include <uv.h>

#include "loop.h"
#include "req.h"
#include "buf.h"

#include "cxx/memory.h"
#include "cxx/defer.h"
#include "cxx/promise.h"

namespace uv {
    class fs_t : public req_extend_t<uv_fs_t, req_t> {
    public:
        using self = fs_t;
        using supper = req_extend_t<uv_fs_t, req_t>;

        [[nodiscard]]
        uv_fs_type get_type() const {
            return uv_fs_get_type(*this);
        }

        [[nodiscard]]
        ssize_t get_result() const {
            return uv_fs_get_result(*this);
        }

        [[nodiscard]]
        int get_system_error() const {
            return uv_fs_get_system_error(*this);
        }

        [[nodiscard]]
        void *get_ptr() const {
            return uv_fs_get_ptr(*this);
        }

        [[nodiscard]]
        const char *get_path() const {
            return uv_fs_get_path(*this);
        }

        uv_stat_t *get_statbuf() {
            return uv_fs_get_statbuf(*this);
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

                using promise_t = uvcxx::promise<T...>;
                using promise_cast_t = uvcxx::promise_cast<uvcxx::promise<T...>, raw_req_t *>;

                // store the instance of `req` to avoid resource release caused by no external reference
                cxx_req_t cxx_req;
                promise_cast_t promise;

                explicit callback_t(cxx_req_t req, typename promise_cast_t::wrapper_t wrapper)
                        : cxx_req(std::move(req)), promise(promise_t(), wrapper) {
                    cxx_req.set_data(this);
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
                        FUNC func, loop_t loop, cxx_req_t req, ARGS &&...args) const {
                    auto *data = new callback_t<T...>(std::move(req), std::move(wrapper));
                    uvcxx::defer delete_data(std::default_delete<callback_t<T...>>(), data);

                    auto err = std::invoke(
                            func,
                            (loop_t::raw_t *) loop,
                            (cxx_req_t::raw_t *) data->cxx_req,
                            std::forward<ARGS>(args)...,
                            callback_t<T...>::raw_callback);

                    if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);
                    delete_data.release();
                    return data->promise.promise();
                }
            };
        }

        inline uvcxx::promise<uv_file> open(loop_t loop, fs_t fs, const char *path, int flags, int mode = 0644) {
            return inner::invoker<uv_file>()(
                    [](inner::raw_req_t *req) { return (uv_file) req->result; },
                    uv_fs_open, std::move(loop), std::move(fs), path, flags, mode);
        }

        inline uvcxx::promise<uv_file> open(const char *path, int flags, int mode = 0644) {
            return open(default_loop(), {}, path, flags, mode);
        }

        inline uvcxx::promise<uv_file> open(loop_t loop, const char *path, int flags, int mode = 0644) {
            return open(std::move(loop), {}, path, flags, mode);
        }

        inline uvcxx::promise<uv_file> open(fs_t fs, const char *path, int flags, int mode = 0644) {
            return open(default_loop(), std::move(fs), path, flags, mode);
        }

        inline uvcxx::promise<int> close(loop_t loop, fs_t fs, uv_file file) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_close, std::move(loop), std::move(fs), file);
        }

        inline uvcxx::promise<int> close(uv_file file) {
            return close(default_loop(), {}, file);
        }

        inline uvcxx::promise<int> close(loop_t loop, uv_file file) {
            return close(std::move(loop), {}, file);
        }

        inline uvcxx::promise<int> close(fs_t fs, uv_file file) {
            return close(default_loop(), std::move(fs), file);
        }

        inline uvcxx::promise<ssize_t> read(
                loop_t loop, fs_t fs,
                uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset = -1) {
            return inner::invoker<ssize_t>()(
                    [](inner::raw_req_t *req) { return (ssize_t) req->result; },
                    uv_fs_read, std::move(loop), std::move(fs), file, bufs, nbufs, offset);
        }

        inline uvcxx::promise<ssize_t> read(
                loop_t loop, fs_t fs,
                uv_file file, buf_t &buf, int64_t offset = -1) {
            return read(std::move(loop), std::move(fs), file, buf, 1, offset);
        }

        inline uvcxx::promise<ssize_t> read(
                loop_t loop, fs_t fs,
                uv_file file, std::vector<buf_t> &bufs, int64_t offset = -1) {
            std::vector<buf_t::raw_t> raw_bufs(bufs.size());
            for (size_t i = 0; i < bufs.size(); ++i) {
                raw_bufs[i] = *(buf_t::raw_t*)bufs[i];
            }
            auto nbufs = (unsigned int)bufs.size();
            return read(std::move(loop), std::move(fs), file, raw_bufs.data(), nbufs, offset);
        }

        inline uvcxx::promise<ssize_t> read(
                uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset = -1) {
            return read(default_loop(), {}, file, bufs, nbufs, offset);
        }

        inline uvcxx::promise<ssize_t> read(
                loop_t loop,
                uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset = -1) {
            return read(std::move(loop), {}, file, bufs, nbufs, offset);
        }

        inline uvcxx::promise<ssize_t> read(
                fs_t fs,
                uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset = -1) {
            return read(default_loop(), std::move(fs), file, bufs, nbufs, offset);
        }

        inline uvcxx::promise<ssize_t> read(
                uv_file file, buf_t &buf, int64_t offset = -1) {
            return read(default_loop(), {}, file, buf, offset);
        }

        inline uvcxx::promise<ssize_t> read(
                loop_t loop,
                uv_file file, buf_t &buf, int64_t offset = -1) {
            return read(std::move(loop), {}, file, buf, offset);
        }

        inline uvcxx::promise<ssize_t> read(
                fs_t fs,
                uv_file file, buf_t &buf, int64_t offset = -1) {
            return read(default_loop(), std::move(fs), file, buf, offset);
        }

        inline uvcxx::promise<ssize_t> read(
                uv_file file, std::vector<buf_t> &bufs, int64_t offset = -1) {
            return read(default_loop(), {}, file, bufs, offset);
        }

        inline uvcxx::promise<ssize_t> read(
                loop_t loop,
                uv_file file, std::vector<buf_t> &bufs, int64_t offset = -1) {
            return read(std::move(loop), {}, file, bufs, offset);
        }

        inline uvcxx::promise<ssize_t> read(
                fs_t fs,
                uv_file file, std::vector<buf_t> &bufs, int64_t offset = -1) {
            return read(default_loop(), std::move(fs), file, bufs, offset);
        }

        inline uvcxx::promise<ssize_t> write(
                loop_t loop, fs_t fs,
                uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset = -1) {
            return inner::invoker<ssize_t>()(
                    [](inner::raw_req_t *req) { return (ssize_t) req->result; },
                    uv_fs_write, std::move(loop), std::move(fs), file, bufs, nbufs, offset);
        }

        inline uvcxx::promise<ssize_t> write(
                loop_t loop, fs_t fs,
                uv_file file, buf_t &buf, int64_t offset = -1) {
            return write(std::move(loop), std::move(fs), file, buf, 1, offset);
        }

        inline uvcxx::promise<ssize_t> write(
                loop_t loop, fs_t fs,
                uv_file file, std::vector<buf_t> &bufs, int64_t offset = -1) {
            std::vector<buf_t::raw_t> raw_bufs(bufs.size());
            for (size_t i = 0; i < bufs.size(); ++i) {
                raw_bufs[i] = *(buf_t::raw_t*)bufs[i];
            }
            auto nbufs = (unsigned int)bufs.size();
            return write(std::move(loop), std::move(fs), file, raw_bufs.data(), nbufs, offset);
        }

        inline uvcxx::promise<ssize_t> write(
                uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset = -1) {
            return write(default_loop(), {}, file, bufs, nbufs, offset);
        }

        inline uvcxx::promise<ssize_t> write(
                loop_t loop,
                uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset = -1) {
            return write(std::move(loop), {}, file, bufs, nbufs, offset);
        }

        inline uvcxx::promise<ssize_t> write(
                fs_t fs,
                uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset = -1) {
            return write(default_loop(), std::move(fs), file, bufs, nbufs, offset);
        }

        inline uvcxx::promise<ssize_t> write(
                uv_file file, buf_t &buf, int64_t offset = -1) {
            return write(default_loop(), {}, file, buf, offset);
        }

        inline uvcxx::promise<ssize_t> write(
                loop_t loop,
                uv_file file, buf_t &buf, int64_t offset = -1) {
            return write(std::move(loop), {}, file, buf, offset);
        }

        inline uvcxx::promise<ssize_t> write(
                fs_t fs,
                uv_file file, buf_t &buf, int64_t offset = -1) {
            return write(default_loop(), std::move(fs), file, buf, offset);
        }

        inline uvcxx::promise<ssize_t> write(
                uv_file file, std::vector<buf_t> &bufs, int64_t offset = -1) {
            return write(default_loop(), {}, file, bufs, offset);
        }

        inline uvcxx::promise<ssize_t> write(
                loop_t loop,
                uv_file file, std::vector<buf_t> &bufs, int64_t offset = -1) {
            return write(std::move(loop), {}, file, bufs, offset);
        }

        inline uvcxx::promise<ssize_t> write(
                fs_t fs,
                uv_file file, std::vector<buf_t> &bufs, int64_t offset = -1) {
            return write(default_loop(), std::move(fs), file, bufs, offset);
        }

        inline uvcxx::promise<int> unlink(loop_t loop, fs_t fs, const char *path) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_unlink, std::move(loop), std::move(fs), path);
        }

        inline uvcxx::promise<int> unlink(const char *path) {
            return unlink(default_loop(), {}, path);
        }

        inline uvcxx::promise<int> unlink(loop_t loop, const char *path) {
            return unlink(std::move(loop), {}, path);
        }

        inline uvcxx::promise<int> unlink(fs_t fs, const char *path) {
            return unlink(default_loop(), std::move(fs), path);
        }

        inline uvcxx::promise<int> mkdir(loop_t loop, fs_t fs, const char *path, int mode = 0755) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_mkdir, std::move(loop), std::move(fs), path, mode);
        }

        inline uvcxx::promise<int> mkdir(const char *path, int mode = 0755) {
            return mkdir(default_loop(), {}, path, mode);
        }

        inline uvcxx::promise<int> mkdir(loop_t loop, const char *path, int mode = 0755) {
            return mkdir(std::move(loop), {}, path, mode);
        }

        inline uvcxx::promise<int> mkdir(fs_t fs, const char *path, int mode = 0755) {
            return mkdir(default_loop(), std::move(fs), path, mode);
        }

        inline uvcxx::promise<const char *> mkdtemp(loop_t loop, fs_t fs, const char *tpl) {
            return inner::invoker<const char *>()(
                    [](inner::raw_req_t *req) { return req->path; },
                    uv_fs_mkdtemp, std::move(loop), std::move(fs), tpl);
        }

        inline uvcxx::promise<const char *> mkdtemp(const char *tpl) {
            return mkdtemp(default_loop(), {}, tpl);
        }

        inline uvcxx::promise<const char *> mkdtemp(loop_t loop, const char *tpl) {
            return mkdtemp(std::move(loop), {}, tpl);
        }

        inline uvcxx::promise<const char *> mkdtemp(fs_t fs, const char *tpl) {
            return mkdtemp(default_loop(), std::move(fs), tpl);
        }

        inline uvcxx::promise<int, const char *> mkstemp(loop_t loop, fs_t fs, const char *tpl) {
            return inner::invoker<int, const char *>()(
                    [](inner::raw_req_t *req) { return std::make_tuple((int) req->result, req->path); },
                    uv_fs_mkstemp, std::move(loop), std::move(fs), tpl);
        }

        inline uvcxx::promise<int, const char *> mkstemp(const char *tpl) {
            return mkstemp(default_loop(), {}, tpl);
        }

        inline uvcxx::promise<int, const char *> mkstemp(loop_t loop, const char *tpl) {
            return mkstemp(std::move(loop), {}, tpl);
        }

        inline uvcxx::promise<int, const char *> mkstemp(fs_t fs, const char *tpl) {
            return mkstemp(default_loop(), std::move(fs), tpl);
        }

        inline uvcxx::promise<int> rmdir(loop_t loop, fs_t fs, const char *path) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_rmdir, std::move(loop), std::move(fs), path);
        }

        inline uvcxx::promise<int> rmdir(const char *path) {
            return rmdir(default_loop(), {}, path);
        }

        inline uvcxx::promise<int> rmdir(loop_t loop, const char *path) {
            return rmdir(std::move(loop), {}, path);
        }

        inline uvcxx::promise<int> rmdir(fs_t fs, const char *path) {
            return rmdir(default_loop(), std::move(fs), path);
        }

        inline uvcxx::promise<uv_dir_t*> opendir(loop_t loop, fs_t fs, const char *path) {
            return inner::invoker<uv_dir_t*>()(
                    [](inner::raw_req_t *req) { return (uv_dir_t*) req->ptr; },
                    uv_fs_opendir, std::move(loop), std::move(fs), path);
        }

        inline uvcxx::promise<uv_dir_t*> opendir(const char *path) {
            return opendir(default_loop(), {}, path);
        }

        inline uvcxx::promise<uv_dir_t*> opendir(loop_t loop, const char *path) {
            return opendir(std::move(loop), {}, path);
        }

        inline uvcxx::promise<uv_dir_t*> opendir(fs_t fs, const char *path) {
            return opendir(default_loop(), std::move(fs), path);
        }

        inline uvcxx::promise<int> closedir(loop_t loop, fs_t fs, uv_dir_t *dir) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_closedir, std::move(loop), std::move(fs), dir);
        }

        inline uvcxx::promise<int> closedir(uv_dir_t *dir) {
            return closedir(default_loop(), {}, dir);
        }

        inline uvcxx::promise<int> closedir(loop_t loop, uv_dir_t *dir) {
            return closedir(std::move(loop), {}, dir);
        }

        inline uvcxx::promise<int> closedir(fs_t fs, uv_dir_t *dir) {
            return closedir(default_loop(), std::move(fs), dir);
        }

        inline uvcxx::promise<uv_stat_t*> stat(loop_t loop, fs_t fs, const char *path) {
            return inner::invoker<uv_stat_t*>()(
                    [](inner::raw_req_t *req) { return &req->statbuf; },
                    uv_fs_stat, std::move(loop), std::move(fs), path);
        }

        inline uvcxx::promise<uv_stat_t*> stat(const char *path) {
            return stat(default_loop(), {}, path);
        }

        inline uvcxx::promise<uv_stat_t*> stat(loop_t loop, const char *path) {
            return stat(std::move(loop), {}, path);
        }

        inline uvcxx::promise<uv_stat_t*> stat(fs_t fs, const char *path) {
            return stat(default_loop(), std::move(fs), path);
        }

        inline uvcxx::promise<uv_stat_t*> fstat(loop_t loop, fs_t fs, uv_file file) {
            return inner::invoker<uv_stat_t*>()(
                    [](inner::raw_req_t *req) { return &req->statbuf; },
                    uv_fs_fstat, std::move(loop), std::move(fs), file);
        }

        inline uvcxx::promise<uv_stat_t*> fstat(uv_file file) {
            return fstat(default_loop(), {}, file);
        }

        inline uvcxx::promise<uv_stat_t*> fstat(loop_t loop, uv_file file) {
            return fstat(std::move(loop), {}, file);
        }

        inline uvcxx::promise<uv_stat_t*> fstat(fs_t fs, uv_file file) {
            return fstat(default_loop(), std::move(fs), file);
        }

        inline uvcxx::promise<uv_stat_t*> lstat(loop_t loop, fs_t fs, const char *path) {
            return inner::invoker<uv_stat_t*>()(
                    [](inner::raw_req_t *req) { return &req->statbuf; },
                    uv_fs_lstat, std::move(loop), std::move(fs), path);
        }

        inline uvcxx::promise<uv_stat_t*> lstat(const char *path) {
            return lstat(default_loop(), {}, path);
        }

        inline uvcxx::promise<uv_stat_t*> lstat(loop_t loop, const char *path) {
            return lstat(std::move(loop), {}, path);
        }

        inline uvcxx::promise<uv_stat_t*> lstat(fs_t fs, const char *path) {
            return lstat(default_loop(), std::move(fs), path);
        }

        inline uvcxx::promise<uv_statfs_t*> statfs(loop_t loop, fs_t fs, const char *path) {
            return inner::invoker<uv_statfs_t*>()(
                    [](inner::raw_req_t *req) { return (uv_statfs_t *) req->ptr; },
                    uv_fs_statfs, std::move(loop), std::move(fs), path);
        }

        inline uvcxx::promise<uv_statfs_t*> statfs(const char *path) {
            return statfs(default_loop(), {}, path);
        }

        inline uvcxx::promise<uv_statfs_t*> statfs(loop_t loop, const char *path) {
            return statfs(std::move(loop), {}, path);
        }

        inline uvcxx::promise<uv_statfs_t*> statfs(fs_t fs, const char *path) {
            return statfs(default_loop(), std::move(fs), path);
        }

        inline uvcxx::promise<int> rename(loop_t loop, fs_t fs, const char *path, const char *new_path) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_rename, std::move(loop), std::move(fs), path, new_path);
        }

        inline uvcxx::promise<int> rename(const char *path, const char *new_path) {
            return rename(default_loop(), {}, path, new_path);
        }

        inline uvcxx::promise<int> rename(loop_t loop, const char *path, const char *new_path) {
            return rename(std::move(loop), {}, path, new_path);
        }

        inline uvcxx::promise<int> rename(fs_t fs, const char *path, const char *new_path) {
            return rename(default_loop(), std::move(fs), path, new_path);
        }

        inline uvcxx::promise<int> fsync(loop_t loop, fs_t fs, uv_file file) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_fsync, std::move(loop), std::move(fs), file);
        }

        inline uvcxx::promise<int> fsync(uv_file file) {
            return fsync(default_loop(), {}, file);
        }

        inline uvcxx::promise<int> fsync(loop_t loop, uv_file file) {
            return fsync(std::move(loop), {}, file);
        }

        inline uvcxx::promise<int> fsync(fs_t fs, uv_file file) {
            return fsync(default_loop(), std::move(fs), file);
        }

        inline uvcxx::promise<int> fdatasync(loop_t loop, fs_t fs, uv_file file) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_fdatasync, std::move(loop), std::move(fs), file);
        }

        inline uvcxx::promise<int> fdatasync(uv_file file) {
            return fdatasync(default_loop(), {}, file);
        }

        inline uvcxx::promise<int> fdatasync(loop_t loop, uv_file file) {
            return fdatasync(std::move(loop), {}, file);
        }

        inline uvcxx::promise<int> fdatasync(fs_t fs, uv_file file) {
            return fdatasync(default_loop(), std::move(fs), file);
        }

        inline uvcxx::promise<int> ftruncate(loop_t loop, fs_t fs, uv_file file, int64_t offset) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_ftruncate, std::move(loop), std::move(fs), file, offset);
        }

        inline uvcxx::promise<int> ftruncate(uv_file file, int64_t offset) {
            return ftruncate(default_loop(), {}, file, offset);
        }

        inline uvcxx::promise<int> ftruncate(loop_t loop, uv_file file, int64_t offset) {
            return ftruncate(std::move(loop), {}, file, offset);
        }

        inline uvcxx::promise<int> ftruncate(fs_t fs, uv_file file, int64_t offset) {
            return ftruncate(default_loop(), std::move(fs), file, offset);
        }

        inline uvcxx::promise<int> copyfile(
                loop_t loop, fs_t fs, const char *path, const char *new_path, int flags) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_copyfile, std::move(loop), std::move(fs), path, new_path, flags);
        }

        inline uvcxx::promise<int> copyfile(const char *path, const char *new_path, int flags) {
            return copyfile(default_loop(), {}, path, new_path, flags);
        }

        inline uvcxx::promise<int> copyfile(loop_t loop, const char *path, const char *new_path, int flags) {
            return copyfile(std::move(loop), {}, path, new_path, flags);
        }

        inline uvcxx::promise<int> copyfile(fs_t fs, const char *path, const char *new_path, int flags) {
            return copyfile(default_loop(), std::move(fs), path, new_path, flags);
        }

        inline uvcxx::promise<ssize_t> sendfile(
                loop_t loop, fs_t fs,
                uv_file out_fd, uv_file in_fd, int64_t in_offset, size_t length) {
            return inner::invoker<ssize_t>()(
                    [](inner::raw_req_t *req) { return (ssize_t) req->result; },
                    uv_fs_sendfile, std::move(loop), std::move(fs), out_fd, in_fd, in_offset, length);
        }

        inline uvcxx::promise<ssize_t> sendfile(
                uv_file out_fd, uv_file in_fd, int64_t in_offset, size_t length) {
            return sendfile(default_loop(), {}, out_fd, in_fd, in_offset, length);
        }

        inline uvcxx::promise<ssize_t> sendfile(
                loop_t loop, uv_file out_fd, uv_file in_fd, int64_t in_offset, size_t length) {
            return sendfile(std::move(loop), {}, out_fd, in_fd, in_offset, length);
        }

        inline uvcxx::promise<ssize_t> sendfile(
                fs_t fs, uv_file out_fd, uv_file in_fd, int64_t in_offset, size_t length) {
            return sendfile(default_loop(), std::move(fs), out_fd, in_fd, in_offset, length);
        }

        inline uvcxx::promise<int> access(loop_t loop, fs_t fs, const char *path, int mode) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_access, std::move(loop), std::move(fs), path, mode);
        }

        inline uvcxx::promise<int> access(const char *path, int mode) {
            return access(default_loop(), {}, path, mode);
        }

        inline uvcxx::promise<int> access(loop_t loop, const char *path, int mode) {
            return access(std::move(loop), {}, path, mode);
        }

        inline uvcxx::promise<int> access(fs_t fs, const char *path, int mode) {
            return access(default_loop(), std::move(fs), path, mode);
        }

        inline uvcxx::promise<int> chmod(loop_t loop, fs_t fs, const char *path, int mode) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_chmod, std::move(loop), std::move(fs), path, mode);
        }

        inline uvcxx::promise<int> chmod(const char *path, int mode) {
            return chmod(default_loop(), {}, path, mode);
        }

        inline uvcxx::promise<int> chmod(loop_t loop, const char *path, int mode) {
            return chmod(std::move(loop), {}, path, mode);
        }

        inline uvcxx::promise<int> chmod(fs_t fs, const char *path, int mode) {
            return chmod(default_loop(), std::move(fs), path, mode);
        }

        inline uvcxx::promise<int> fchmod(loop_t loop, fs_t fs, uv_file file, int mode) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_fchmod, std::move(loop), std::move(fs), file, mode);
        }

        inline uvcxx::promise<int> fchmod(uv_file file, int mode) {
            return fchmod(default_loop(), {}, file, mode);
        }

        inline uvcxx::promise<int> fchmod(loop_t loop, uv_file file, int mode) {
            return fchmod(std::move(loop), {}, file, mode);
        }

        inline uvcxx::promise<int> fchmod(fs_t fs, uv_file file, int mode) {
            return fchmod(default_loop(), std::move(fs), file, mode);
        }

        inline uvcxx::promise<int> utime(loop_t loop, fs_t fs, const char *path, double atime, double mtime) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_utime, std::move(loop), std::move(fs), path, atime, mtime);
        }

        inline uvcxx::promise<int> utime(const char *path, double atime, double mtime) {
            return utime(default_loop(), {}, path, atime, mtime);
        }

        inline uvcxx::promise<int> utime(loop_t loop, const char *path, double atime, double mtime) {
            return utime(std::move(loop), {}, path, atime, mtime);
        }

        inline uvcxx::promise<int> utime(fs_t fs, const char *path, double atime, double mtime) {
            return utime(default_loop(), std::move(fs), path, atime, mtime);
        }

        inline uvcxx::promise<int> futime(loop_t loop, fs_t fs, uv_file file, double atime, double mtime) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_futime, std::move(loop), std::move(fs), file, atime, mtime);
        }

        inline uvcxx::promise<int> futime(uv_file file, double atime, double mtime) {
            return futime(default_loop(), {}, file, atime, mtime);
        }

        inline uvcxx::promise<int> futime(loop_t loop, uv_file file, double atime, double mtime) {
            return futime(std::move(loop), {}, file, atime, mtime);
        }

        inline uvcxx::promise<int> futime(fs_t fs, uv_file file, double atime, double mtime) {
            return futime(default_loop(), std::move(fs), file, atime, mtime);
        }

        inline uvcxx::promise<int> lutime(loop_t loop, fs_t fs, const char *path, double atime, double mtime) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_lutime, std::move(loop), std::move(fs), path, atime, mtime);
        }

        inline uvcxx::promise<int> lutime(const char *path, double atime, double mtime) {
            return lutime(default_loop(), {}, path, atime, mtime);
        }

        inline uvcxx::promise<int> lutime(loop_t loop, const char *path, double atime, double mtime) {
            return lutime(std::move(loop), {}, path, atime, mtime);
        }

        inline uvcxx::promise<int> lutime(fs_t fs, const char *path, double atime, double mtime) {
            return lutime(default_loop(), std::move(fs), path, atime, mtime);
        }

        inline uvcxx::promise<int> link(
                loop_t loop, fs_t fs, const char *path, const char *new_path) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_link, std::move(loop), std::move(fs), path, new_path);
        }

        inline uvcxx::promise<int> link(const char *path, const char *new_path) {
            return link(default_loop(), {}, path, new_path);
        }

        inline uvcxx::promise<int> link(loop_t loop, const char *path, const char *new_path) {
            return link(std::move(loop), {}, path, new_path);
        }

        inline uvcxx::promise<int> link(fs_t fs, const char *path, const char *new_path) {
            return link(default_loop(), std::move(fs), path, new_path);
        }

        inline uvcxx::promise<int> symlink(
                loop_t loop, fs_t fs, const char *path, const char *new_path, int flags) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_symlink, std::move(loop), std::move(fs), path, new_path, flags);
        }

        inline uvcxx::promise<int> symlink(const char *path, const char *new_path, int flags) {
            return symlink(default_loop(), {}, path, new_path, flags);
        }

        inline uvcxx::promise<int> symlink(loop_t loop, const char *path, const char *new_path, int flags) {
            return symlink(std::move(loop), {}, path, new_path, flags);
        }

        inline uvcxx::promise<int> symlink(fs_t fs, const char *path, const char *new_path, int flags) {
            return symlink(default_loop(), std::move(fs), path, new_path, flags);
        }

        inline uvcxx::promise<const char *> readlink(loop_t loop, fs_t fs, const char *path) {
            return inner::invoker<const char *>()(
                    [](inner::raw_req_t *req) { return (const char *) req->ptr; },
                    uv_fs_readlink, std::move(loop), std::move(fs), path);
        }

        inline uvcxx::promise<const char *> readlink(const char *path) {
            return readlink(default_loop(), {}, path);
        }

        inline uvcxx::promise<const char *> readlink(loop_t loop, const char *path) {
            return readlink(std::move(loop), {}, path);
        }

        inline uvcxx::promise<const char *> readlink(fs_t fs, const char *path) {
            return readlink(default_loop(), std::move(fs), path);
        }

        inline uvcxx::promise<const char *> realpath(loop_t loop, fs_t fs, const char *path) {
            return inner::invoker<const char *>()(
                    [](inner::raw_req_t *req) { return (const char *) req->ptr; },
                    uv_fs_realpath, std::move(loop), std::move(fs), path);
        }

        inline uvcxx::promise<const char *> realpath(const char *path) {
            return realpath(default_loop(), {}, path);
        }

        inline uvcxx::promise<const char *> realpath(loop_t loop, const char *path) {
            return realpath(std::move(loop), {}, path);
        }

        inline uvcxx::promise<const char *> realpath(fs_t fs, const char *path) {
            return realpath(default_loop(), std::move(fs), path);
        }

        inline uvcxx::promise<int> chown(
                loop_t loop, fs_t fs, const char *path, uv_uid_t uid, uv_gid_t gid) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_chown, std::move(loop), std::move(fs), path, uid, gid);
        }

        inline uvcxx::promise<int> chown(const char *path, uv_uid_t uid, uv_gid_t gid) {
            return chown(default_loop(), {}, path, uid, gid);
        }

        inline uvcxx::promise<int> chown(loop_t loop, const char *path, uv_uid_t uid, uv_gid_t gid) {
            return chown(std::move(loop), {}, path, uid, gid);
        }

        inline uvcxx::promise<int> chown(fs_t fs, const char *path, uv_uid_t uid, uv_gid_t gid) {
            return chown(default_loop(), std::move(fs), path, uid, gid);
        }

        inline uvcxx::promise<int> fchown(
                loop_t loop, fs_t fs, uv_file file, uv_uid_t uid, uv_gid_t gid) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_fchown, std::move(loop), std::move(fs), file, uid, gid);
        }

        inline uvcxx::promise<int> fchown(uv_file file, uv_uid_t uid, uv_gid_t gid) {
            return fchown(default_loop(), {}, file, uid, gid);
        }

        inline uvcxx::promise<int> fchown(loop_t loop, uv_file file, uv_uid_t uid, uv_gid_t gid) {
            return fchown(std::move(loop), {}, file, uid, gid);
        }

        inline uvcxx::promise<int> fchown(fs_t fs, uv_file file, uv_uid_t uid, uv_gid_t gid) {
            return fchown(default_loop(), std::move(fs), file, uid, gid);
        }

        inline uvcxx::promise<int> lchown(
                loop_t loop, fs_t fs, const char *path, uv_uid_t uid, uv_gid_t gid) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_lchown, std::move(loop), std::move(fs), path, uid, gid);
        }

        inline uvcxx::promise<int> lchown(const char *path, uv_uid_t uid, uv_gid_t gid) {
            return lchown(default_loop(), {}, path, uid, gid);
        }

        inline uvcxx::promise<int> lchown(loop_t loop, const char *path, uv_uid_t uid, uv_gid_t gid) {
            return lchown(std::move(loop), {}, path, uid, gid);
        }

        inline uvcxx::promise<int> lchown(fs_t fs, const char *path, uv_uid_t uid, uv_gid_t gid) {
            return lchown(default_loop(), std::move(fs), path, uid, gid);
        }
    }
}


#endif //LIBUVCXX_FS_H
