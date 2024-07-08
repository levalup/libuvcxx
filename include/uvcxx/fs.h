//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_FS_H
#define LIBUVCXX_FS_H

#include <vector>

#include "cxx/buffer.h"
#include "cxx/string.h"

#include "buf.h"
#include "loop.h"
#include "req.h"

namespace uv {
    class fs_t : public inherit_req_t<uv_fs_t, req_t> {
    public:
        using self = fs_t;
        using supper = inherit_req_t<uv_fs_t, req_t>;

        UVCXX_NODISCARD
        loop_t loop() const {
            return loop_t::borrow(raw<raw_t>()->loop);
        }

        UVCXX_NODISCARD
        uv_fs_type fs_type() const {
            return raw<raw_t>()->fs_type;
        }

        UVCXX_NODISCARD
        const char *path() const {
            return raw<raw_t>()->path;
        }

        UVCXX_NODISCARD
        ssize_t result() const {
            return raw<raw_t>()->result;
        }

        UVCXX_NODISCARD
        uv_stat_t statbuf() const {
            return raw<raw_t>()->statbuf;
        }

        UVCXX_NODISCARD
        void *ptr() const {
            return raw<raw_t>()->ptr;
        }

        void cleanup() {
            uv_fs_req_cleanup(*this);
        }

        UVCXX_NODISCARD
        uv_fs_type get_fs_type() const {
            // cover uv_fs_get_type
            return raw<raw_t>()->fs_type;
        }

        UVCXX_NODISCARD
        ssize_t get_result() const {
            // cover uv_fs_get_result
            return raw<raw_t>()->result;
        }

#if UVCXX_SATISFY_VERSION(1, 38, 0)

        UVCXX_NODISCARD
        int get_system_error() const {
            return uv_fs_get_system_error(*this);
        }

#endif

        UVCXX_NODISCARD
        void *get_ptr() const {
            // cover uv_fs_get_ptr
            return raw<raw_t>()->ptr;
        }

        UVCXX_NODISCARD
        const char *get_path() const {
            // cover uv_fs_get_path
            return raw<raw_t>()->path;
        }

        uv_stat_t *get_statbuf() {
            // cover uv_fs_get_statbuf
            return &raw<raw_t>()->statbuf;
        }

        template<typename T>
        UVCXX_NODISCARD
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

                template<typename WRAPPER, typename std::enable_if<
                        std::is_constructible<promise_cast_t, WRAPPER>::value, int>::type = 0>
                explicit callback_t(const cxx_req_t &req, WRAPPER wrapper)
                        : supper(req), promise(std::move(wrapper)) {
                }

                uvcxx::promise_proxy<raw_req_t *> &proxy() UVCXX_NOEXCEPT final {
                    return promise;
                }

                void finalize(raw_req_t *req) UVCXX_NOEXCEPT final {
                    uv_fs_req_cleanup(req);
                }

                int check(raw_req_t *req) UVCXX_NOEXCEPT final {
                    return (int) req->result;
                }

                void *malloc(size_t size) {
                    if (!m_buf) m_buf = buf_t(size);
                    else m_buf.malloc(size);
                    return m_buf.data();
                }

                template<typename K>
                K *calloc(size_t count) {
                    auto size = count * sizeof(K);
                    if (!m_buf) m_buf = buf_t(size);
                    else m_buf.malloc(size);
                    return (K *) m_buf.data();
                }

            private:
                buf_t m_buf;
            };

            template<typename... T>
            class invoker {
            public:
                invoker() = default;

                explicit invoker(std::function<void(callback_t<T...> *)> init) : init(std::move(init)) {}

                template<typename WRAPPER, typename FUNC, typename ...ARGS>
                uvcxx::promise<T...> operator()(
                        WRAPPER wrapper,
                        FUNC func, const loop_t &loop, const cxx_req_t &req, ARGS &&...args) const {
                    auto *data = new callback_t<T...>(req, std::move(wrapper));
                    uvcxx::defer_delete<callback_t<T...>> delete_data(data);

                    if (init) init(data);

                    UVCXX_APPLY(func(
                            (loop_t::raw_t *) loop,
                            (cxx_req_t::raw_t *) data->req(),
                            std::forward<ARGS>(args)...,
                            callback_t<T...>::raw_callback), nullptr);

                    delete_data.release();
                    ((raw_req_t *) req)->data = data;
                    return data->promise.promise();
                }

            private:
                std::function<void(callback_t<T...> *)> init;
            };
        }

        inline int open(std::nullptr_t, fs_t &req,
                        uvcxx::string path, int flags, int mode,
                        std::nullptr_t) {
            UVCXX_PROXY(uv_fs_open(nullptr, req, path, flags, mode, nullptr));
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<uv_file> open(
                const loop_t &loop, const fs_t &req, uvcxx::string path, int flags, int mode = 0644) {
            return inner::invoker<uv_file>()(
                    [](inner::raw_req_t *req) { return (uv_file) req->result; },
                    uv_fs_open, loop, req, path, flags, mode);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<uv_file> open(uvcxx::string path, int flags, int mode = 0644) {
            return open(default_loop(), {}, path, flags, mode);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<uv_file> open(const loop_t &loop, uvcxx::string path, int flags, int mode = 0644) {
            return open(loop, {}, path, flags, mode);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<uv_file> open(const fs_t &req, uvcxx::string path, int flags, int mode = 0644) {
            return open(default_loop(), req, path, flags, mode);
        }

        inline int close(std::nullptr_t, fs_t &req,
                         uv_file file,
                         std::nullptr_t) {
            UVCXX_PROXY(uv_fs_close(nullptr, req, file, nullptr));
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> close(const loop_t &loop, const fs_t &req, uv_file file) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_close, loop, req, file);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> close(uv_file file) {
            return close(default_loop(), {}, file);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> close(const loop_t &loop, uv_file file) {
            return close(loop, {}, file);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> close(const fs_t &req, uv_file file) {
            return close(default_loop(), req, file);
        }

        inline int read(std::nullptr_t, fs_t &req,
                        uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset,
                        std::nullptr_t) {
            UVCXX_PROXY(uv_fs_read(nullptr, req, file, bufs, nbufs, offset, nullptr));
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<ssize_t> read(
                const loop_t &loop, const fs_t &req,
                uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset) {
            return inner::invoker<ssize_t>()(
                    [](inner::raw_req_t *req) { return (ssize_t) req->result; },
                    uv_fs_read, loop, req, file, bufs, nbufs, offset);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<ssize_t> read(
                uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset) {
            return read(default_loop(), {}, file, bufs, nbufs, offset);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<ssize_t> read(
                const loop_t &loop,
                uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset) {
            return read(loop, {}, file, bufs, nbufs, offset);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<ssize_t> read(
                const fs_t &req,
                uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset) {
            return read(default_loop(), req, file, bufs, nbufs, offset);
        }

        inline int read(std::nullptr_t, fs_t &req,
                        uv_file file, uvcxx::mutable_buffer buf, int64_t offset,
                        std::nullptr_t) {
            return read(nullptr, req, file, &buf.buf, 1, offset, nullptr);
        }

        inline int read(std::nullptr_t, fs_t &req,
                        uv_file file, std::initializer_list<uvcxx::mutable_buffer> bufs, int64_t offset,
                        std::nullptr_t) {
            std::vector<uv_buf_t> buffers;
            buffers.reserve(bufs.size());
            for (auto &buf: bufs) { buffers.emplace_back(buf.buf); }
            return read(nullptr, req, file, buffers.data(), (unsigned int) buffers.size(), offset, nullptr);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<ssize_t> read(
                const loop_t &loop, const fs_t &req,
                uv_file file, uvcxx::mutable_buffer buf, int64_t offset) {
            return read(loop, req, file, &buf.buf, 1, offset);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<ssize_t> read(
                const loop_t &loop, const fs_t &req,
                uv_file file, std::initializer_list<uvcxx::mutable_buffer> bufs, int64_t offset) {
            std::vector<uv_buf_t> buffers;
            buffers.reserve(bufs.size());
            for (auto &buf: bufs) { buffers.emplace_back(buf.buf); }
            return read(loop, req, file, buffers.data(), (unsigned int) buffers.size(), offset);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<ssize_t> read(
                uv_file file, uvcxx::mutable_buffer buf, int64_t offset) {
            return read(file, &buf.buf, 1, offset);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<ssize_t> read(
                uv_file file, std::initializer_list<uvcxx::mutable_buffer> bufs, int64_t offset) {
            std::vector<uv_buf_t> buffers;
            buffers.reserve(bufs.size());
            for (auto &buf: bufs) { buffers.emplace_back(buf.buf); }
            return read(file, buffers.data(), (unsigned int) buffers.size(), offset);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<ssize_t> read(
                const loop_t &loop,
                uv_file file, uvcxx::mutable_buffer buf, int64_t offset) {
            return read(loop, file, &buf.buf, 1, offset);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<ssize_t> read(
                const loop_t &loop,
                uv_file file, std::initializer_list<uvcxx::mutable_buffer> bufs, int64_t offset) {
            std::vector<uv_buf_t> buffers;
            buffers.reserve(bufs.size());
            for (auto &buf: bufs) { buffers.emplace_back(buf.buf); }
            return read(loop, file, buffers.data(), (unsigned int) buffers.size(), offset);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<ssize_t> read(
                const fs_t &req,
                uv_file file, uvcxx::mutable_buffer buf, int64_t offset) {
            return read(req, file, &buf.buf, 1, offset);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<ssize_t> read(
                const fs_t &req,
                uv_file file, std::initializer_list<uvcxx::mutable_buffer> bufs, int64_t offset) {
            std::vector<uv_buf_t> buffers;
            buffers.reserve(bufs.size());
            for (auto &buf: bufs) { buffers.emplace_back(buf.buf); }
            return read(req, file, buffers.data(), (unsigned int) buffers.size(), offset);
        }

        inline int write(std::nullptr_t, fs_t &req,
                         uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset,
                         std::nullptr_t) {
            UVCXX_PROXY(uv_fs_write(nullptr, req, file, bufs, nbufs, offset, nullptr));
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<ssize_t> write(
                const loop_t &loop, const fs_t &req,
                uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset) {
            return inner::invoker<ssize_t>()(
                    [](inner::raw_req_t *req) { return (ssize_t) req->result; },
                    uv_fs_write, loop, req, file, bufs, nbufs, offset);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<ssize_t> write(
                uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset) {
            return write(default_loop(), {}, file, bufs, nbufs, offset);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<ssize_t> write(
                const loop_t &loop,
                uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset) {
            return write(loop, {}, file, bufs, nbufs, offset);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<ssize_t> write(
                const fs_t &req,
                uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset) {
            return write(default_loop(), req, file, bufs, nbufs, offset);
        }

        inline int write(std::nullptr_t, fs_t &req,
                         uv_file file, uvcxx::buffer buf, int64_t offset,
                         std::nullptr_t) {
            return write(nullptr, req, file, &buf.buf, 1, offset, nullptr);
        }

        inline int write(std::nullptr_t, fs_t &req,
                         uv_file file, std::initializer_list<uvcxx::buffer> bufs, int64_t offset,
                         std::nullptr_t) {
            std::vector<uv_buf_t> buffers;
            buffers.reserve(bufs.size());
            for (auto &buf: bufs) { buffers.emplace_back(buf.buf); }
            return write(nullptr, req, file, buffers.data(), (unsigned int) buffers.size(), offset, nullptr);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<ssize_t> write(
                const loop_t &loop, const fs_t &req,
                uv_file file, uvcxx::buffer buf, int64_t offset) {
            return write(loop, req, file, &buf.buf, 1, offset);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<ssize_t> write(
                const loop_t &loop, const fs_t &req,
                uv_file file, std::initializer_list<uvcxx::buffer> bufs, int64_t offset) {
            std::vector<uv_buf_t> buffers;
            buffers.reserve(bufs.size());
            for (auto &buf: bufs) { buffers.emplace_back(buf.buf); }
            return write(loop, req, file, buffers.data(), (unsigned int) buffers.size(), offset);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<ssize_t> write(
                uv_file file, uvcxx::buffer buf, int64_t offset) {
            return write(file, &buf.buf, 1, offset);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<ssize_t> write(
                uv_file file, std::initializer_list<uvcxx::buffer> bufs, int64_t offset) {
            std::vector<uv_buf_t> buffers;
            buffers.reserve(bufs.size());
            for (auto &buf: bufs) { buffers.emplace_back(buf.buf); }
            return write(file, buffers.data(), (unsigned int) buffers.size(), offset);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<ssize_t> write(
                const loop_t &loop,
                uv_file file, uvcxx::buffer buf, int64_t offset) {
            return write(loop, file, &buf.buf, 1, offset);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<ssize_t> write(
                const loop_t &loop,
                uv_file file, std::initializer_list<uvcxx::buffer> bufs, int64_t offset) {
            std::vector<uv_buf_t> buffers;
            buffers.reserve(bufs.size());
            for (auto &buf: bufs) { buffers.emplace_back(buf.buf); }
            return write(loop, file, buffers.data(), (unsigned int) buffers.size(), offset);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<ssize_t> write(
                const fs_t &req,
                uv_file file, uvcxx::buffer buf, int64_t offset) {
            return write(req, file, &buf.buf, 1, offset);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<ssize_t> write(
                const fs_t &req,
                uv_file file, std::initializer_list<uvcxx::buffer> bufs, int64_t offset) {
            std::vector<uv_buf_t> buffers;
            buffers.reserve(bufs.size());
            for (auto &buf: bufs) { buffers.emplace_back(buf.buf); }
            return write(req, file, buffers.data(), (unsigned int) buffers.size(), offset);
        }

        inline int unlink(std::nullptr_t, fs_t &req,
                          uvcxx::string path,
                          std::nullptr_t) {
            UVCXX_PROXY(uv_fs_unlink(nullptr, req, path, nullptr));
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> unlink(const loop_t &loop, const fs_t &req, uvcxx::string path) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_unlink, loop, req, path);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> unlink(uvcxx::string path) {
            return unlink(default_loop(), {}, path);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> unlink(const loop_t &loop, uvcxx::string path) {
            return unlink(loop, {}, path);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> unlink(const fs_t &req, uvcxx::string path) {
            return unlink(default_loop(), req, path);
        }

        inline int mkdir(std::nullptr_t, fs_t &req,
                         uvcxx::string path, int mode,
                         std::nullptr_t) {
            auto status = uv_fs_mkdir(nullptr, req, path, mode, nullptr);
            if (status == UV_EEXIST) return status; //< exist ok
            if (status < 0) UVCXX_THROW_OR_RETURN(status, status);
            return status;
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> mkdir(const loop_t &loop, const fs_t &req, uvcxx::string path, int mode = 0755) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_mkdir, loop, req, path, mode);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> mkdir(uvcxx::string path, int mode = 0755) {
            return mkdir(default_loop(), {}, path, mode);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> mkdir(const loop_t &loop, uvcxx::string path, int mode = 0755) {
            return mkdir(loop, {}, path, mode);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> mkdir(const fs_t &req, uvcxx::string path, int mode = 0755) {
            return mkdir(default_loop(), req, path, mode);
        }

        inline int mkdtemp(std::nullptr_t, fs_t &req,
                           uvcxx::string tpl,
                           std::nullptr_t) {
            UVCXX_PROXY(uv_fs_mkdtemp(nullptr, req, tpl, nullptr));
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<const char *> mkdtemp(const loop_t &loop, const fs_t &req, uvcxx::string tpl) {
            return inner::invoker<const char *>()(
                    [](inner::raw_req_t *req) { return req->path; },
                    uv_fs_mkdtemp, loop, req, tpl);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<const char *> mkdtemp(uvcxx::string tpl) {
            return mkdtemp(default_loop(), {}, tpl);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<const char *> mkdtemp(const loop_t &loop, uvcxx::string tpl) {
            return mkdtemp(loop, {}, tpl);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<const char *> mkdtemp(const fs_t &req, uvcxx::string tpl) {
            return mkdtemp(default_loop(), req, tpl);
        }

#if UVCXX_SATISFY_VERSION(1, 34, 0)

        inline int mkstemp(std::nullptr_t, fs_t &req,
                           uvcxx::string tpl,
                           std::nullptr_t) {
            UVCXX_PROXY(uv_fs_mkstemp(nullptr, req, tpl, nullptr));
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int, const char *> mkstemp(const loop_t &loop, const fs_t &req, uvcxx::string tpl) {
            return inner::invoker<int, const char *>()(
                    [](inner::raw_req_t *req) { return std::make_tuple((int) req->result, req->path); },
                    uv_fs_mkstemp, loop, req, tpl);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int, const char *> mkstemp(uvcxx::string tpl) {
            return mkstemp(default_loop(), {}, tpl);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int, const char *> mkstemp(const loop_t &loop, uvcxx::string tpl) {
            return mkstemp(loop, {}, tpl);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int, const char *> mkstemp(const fs_t &req, uvcxx::string tpl) {
            return mkstemp(default_loop(), req, tpl);
        }

#endif

        inline int rmdir(std::nullptr_t, fs_t &req,
                         uvcxx::string path,
                         std::nullptr_t) {
            UVCXX_PROXY(uv_fs_rmdir(nullptr, req, path, nullptr));
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> rmdir(const loop_t &loop, const fs_t &req, uvcxx::string path) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_rmdir, loop, req, path);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> rmdir(uvcxx::string path) {
            return rmdir(default_loop(), {}, path);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> rmdir(const loop_t &loop, uvcxx::string path) {
            return rmdir(loop, {}, path);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> rmdir(const fs_t &req, uvcxx::string path) {
            return rmdir(default_loop(), req, path);
        }

#if UVCXX_SATISFY_VERSION(1, 28, 0)

        inline int opendir(std::nullptr_t, fs_t &req,
                           uvcxx::string path,
                           std::nullptr_t) {
            UVCXX_PROXY(uv_fs_opendir(nullptr, req, path, nullptr));
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<uv_dir_t *> opendir(const loop_t &loop, const fs_t &req, uvcxx::string path) {
            return inner::invoker<uv_dir_t *>()(
                    [](inner::raw_req_t *req) { return (uv_dir_t *) req->ptr; },
                    uv_fs_opendir, loop, req, path);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<uv_dir_t *> opendir(uvcxx::string path) {
            return opendir(default_loop(), {}, path);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<uv_dir_t *> opendir(const loop_t &loop, uvcxx::string path) {
            return opendir(loop, {}, path);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<uv_dir_t *> opendir(const fs_t &req, uvcxx::string path) {
            return opendir(default_loop(), req, path);
        }

        inline int closedir(std::nullptr_t, fs_t &req,
                            uv_dir_t *dir,
                            std::nullptr_t) {
            UVCXX_PROXY(uv_fs_closedir(nullptr, req, dir, nullptr));
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> closedir(const loop_t &loop, const fs_t &req, uv_dir_t *dir) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_closedir, loop, req, dir);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> closedir(uv_dir_t *dir) {
            return closedir(default_loop(), {}, dir);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> closedir(const loop_t &loop, uv_dir_t *dir) {
            return closedir(loop, {}, dir);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> closedir(const fs_t &req, uv_dir_t *dir) {
            return closedir(default_loop(), req, dir);
        }

        inline int readdir(std::nullptr_t, fs_t &req,
                           uv_dir_t *dir,
                           std::nullptr_t) {
            UVCXX_PROXY(uv_fs_readdir(nullptr, req, dir, nullptr));
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<uv_dirent_t *, size_t> readdir(
                const loop_t &loop, const fs_t &req, uv_dir_t *dir, size_t nentries = 256) {
            return inner::invoker<uv_dirent_t *, size_t>([&](inner::callback_t<uv_dirent_t *, size_t> *data) {
                dir->nentries = nentries;
                dir->dirents = data->calloc<uv_dirent_t>(nentries);
            })([](inner::raw_req_t *req) {
                   auto dir = (uv_dir_t *) req->ptr;
                   return std::make_tuple(dir->dirents, size_t(req->result));
               },
               uv_fs_readdir, loop, req, dir);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<uv_dirent_t *, size_t> readdir(uv_dir_t *dir, size_t nentries = 256) {
            return readdir(default_loop(), {}, dir, nentries);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<uv_dirent_t *, size_t> readdir(const loop_t &loop, uv_dir_t *dir, size_t nentries = 256) {
            return readdir(loop, {}, dir, nentries);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<uv_dirent_t *, size_t> readdir(const fs_t &req, uv_dir_t *dir, size_t nentries = 256) {
            return readdir(default_loop(), req, dir, nentries);
        }

#endif

        inline int scandir(std::nullptr_t, fs_t &req,
                           uvcxx::string path, int flags,
                           std::nullptr_t) {
            UVCXX_PROXY(uv_fs_scandir(nullptr, req, path, flags, nullptr));
        }

        using scan_next = std::function<int(uv_dirent_t *ent)>;

        UVCXX_NODISCARD
        inline uvcxx::promise<scan_next> scandir(const loop_t &loop, const fs_t &req, uvcxx::string path, int flags) {
            return inner::invoker<scan_next>()(
                    [](inner::raw_req_t *req) {
                        return [req](uv_dirent_t *ent) -> int {
                            return uv_fs_scandir_next(req, ent);
                        };
                    },
                    uv_fs_scandir, loop, req, path, flags);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<scan_next> scandir(uvcxx::string path, int flags) {
            return scandir(default_loop(), {}, path, flags);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<scan_next> scandir(const loop_t &loop, uvcxx::string path, int flags) {
            return scandir(loop, {}, path, flags);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<scan_next> scandir(const fs_t &req, uvcxx::string path, int flags) {
            return scandir(default_loop(), req, path, flags);
        }

        UVCXX_NODISCARD
        inline int scandir_next(const fs_t &req, uv_dirent_t *ent) {
            // This function is preserved for the uniformity of the interface, but it is not recommended to use.
            // scandir will return a suitable next function to replace this function.
            return uv_fs_scandir_next(req, ent);
        }

        inline int stat(std::nullptr_t, fs_t &req,
                        uvcxx::string path,
                        std::nullptr_t) {
            UVCXX_PROXY(uv_fs_stat(nullptr, req, path, nullptr));
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<uv_stat_t *> stat(const loop_t &loop, const fs_t &req, uvcxx::string path) {
            return inner::invoker<uv_stat_t *>()(
                    [](inner::raw_req_t *req) { return &req->statbuf; },
                    uv_fs_stat, loop, req, path);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<uv_stat_t *> stat(uvcxx::string path) {
            return stat(default_loop(), {}, path);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<uv_stat_t *> stat(const loop_t &loop, uvcxx::string path) {
            return stat(loop, {}, path);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<uv_stat_t *> stat(const fs_t &req, uvcxx::string path) {
            return stat(default_loop(), req, path);
        }

        inline int fstat(std::nullptr_t, fs_t &req,
                         uv_file file,
                         std::nullptr_t) {
            UVCXX_PROXY(uv_fs_fstat(nullptr, req, file, nullptr));
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<uv_stat_t *> fstat(const loop_t &loop, const fs_t &req, uv_file file) {
            return inner::invoker<uv_stat_t *>()(
                    [](inner::raw_req_t *req) { return &req->statbuf; },
                    uv_fs_fstat, loop, req, file);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<uv_stat_t *> fstat(uv_file file) {
            return fstat(default_loop(), {}, file);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<uv_stat_t *> fstat(const loop_t &loop, uv_file file) {
            return fstat(loop, {}, file);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<uv_stat_t *> fstat(const fs_t &req, uv_file file) {
            return fstat(default_loop(), req, file);
        }

        inline int lstat(std::nullptr_t, fs_t &req,
                         uvcxx::string path,
                         std::nullptr_t) {
            UVCXX_PROXY(uv_fs_lstat(nullptr, req, path, nullptr));
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<uv_stat_t *> lstat(const loop_t &loop, const fs_t &req, uvcxx::string path) {
            return inner::invoker<uv_stat_t *>()(
                    [](inner::raw_req_t *req) { return &req->statbuf; },
                    uv_fs_lstat, loop, req, path);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<uv_stat_t *> lstat(uvcxx::string path) {
            return lstat(default_loop(), {}, path);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<uv_stat_t *> lstat(const loop_t &loop, uvcxx::string path) {
            return lstat(loop, {}, path);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<uv_stat_t *> lstat(const fs_t &req, uvcxx::string path) {
            return lstat(default_loop(), req, path);
        }

#if UVCXX_SATISFY_VERSION(1, 31, 0)

        inline int statfs(std::nullptr_t, fs_t &req,
                          uvcxx::string path,
                          std::nullptr_t) {
            UVCXX_PROXY(uv_fs_statfs(nullptr, req, path, nullptr));
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<uv_statfs_t *> statfs(const loop_t &loop, const fs_t &req, uvcxx::string path) {
            return inner::invoker<uv_statfs_t *>()(
                    [](inner::raw_req_t *req) { return (uv_statfs_t *) req->ptr; },
                    uv_fs_statfs, loop, req, path);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<uv_statfs_t *> statfs(uvcxx::string path) {
            return statfs(default_loop(), {}, path);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<uv_statfs_t *> statfs(const loop_t &loop, uvcxx::string path) {
            return statfs(loop, {}, path);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<uv_statfs_t *> statfs(const fs_t &req, uvcxx::string path) {
            return statfs(default_loop(), req, path);
        }

#endif

        inline int rename(std::nullptr_t, fs_t &req,
                          uvcxx::string path, uvcxx::string new_path,
                          std::nullptr_t) {
            UVCXX_PROXY(uv_fs_rename(nullptr, req, path, new_path, nullptr));
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> rename(
                const loop_t &loop, const fs_t &req, uvcxx::string path, uvcxx::string new_path) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_rename, loop, req, path, new_path);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> rename(uvcxx::string path, uvcxx::string new_path) {
            return rename(default_loop(), {}, path, new_path);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> rename(const loop_t &loop, uvcxx::string path, uvcxx::string new_path) {
            return rename(loop, {}, path, new_path);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> rename(const fs_t &req, uvcxx::string path, uvcxx::string new_path) {
            return rename(default_loop(), req, path, new_path);
        }

        inline int fsync(std::nullptr_t, fs_t &req,
                         uv_file file,
                         std::nullptr_t) {
            UVCXX_PROXY(uv_fs_fsync(nullptr, req, file, nullptr));
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> fsync(const loop_t &loop, const fs_t &req, uv_file file) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_fsync, loop, req, file);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> fsync(uv_file file) {
            return fsync(default_loop(), {}, file);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> fsync(const loop_t &loop, uv_file file) {
            return fsync(loop, {}, file);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> fsync(const fs_t &req, uv_file file) {
            return fsync(default_loop(), req, file);
        }

        inline int fdatasync(std::nullptr_t, fs_t &req,
                             uv_file file,
                             std::nullptr_t) {
            UVCXX_PROXY(uv_fs_fdatasync(nullptr, req, file, nullptr));
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> fdatasync(const loop_t &loop, const fs_t &req, uv_file file) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_fdatasync, loop, req, file);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> fdatasync(uv_file file) {
            return fdatasync(default_loop(), {}, file);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> fdatasync(const loop_t &loop, uv_file file) {
            return fdatasync(loop, {}, file);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> fdatasync(const fs_t &req, uv_file file) {
            return fdatasync(default_loop(), req, file);
        }

        inline int ftruncate(std::nullptr_t, fs_t &req,
                             uv_file file, int64_t offset,
                             std::nullptr_t) {
            UVCXX_PROXY(uv_fs_ftruncate(nullptr, req, file, offset, nullptr));
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> ftruncate(
                const loop_t &loop, const fs_t &req, uv_file file, int64_t offset) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_ftruncate, loop, req, file, offset);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> ftruncate(uv_file file, int64_t offset) {
            return ftruncate(default_loop(), {}, file, offset);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> ftruncate(const loop_t &loop, uv_file file, int64_t offset) {
            return ftruncate(loop, {}, file, offset);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> ftruncate(const fs_t &req, uv_file file, int64_t offset) {
            return ftruncate(default_loop(), req, file, offset);
        }

#if UVCXX_SATISFY_VERSION(1, 14, 0)

        inline int copyfile(std::nullptr_t, fs_t &req,
                            uvcxx::string path, uvcxx::string new_path, int flags,
                            std::nullptr_t) {
            UVCXX_PROXY(uv_fs_copyfile(nullptr, req, path, new_path, flags, nullptr));
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> copyfile(
                const loop_t &loop, const fs_t &req, uvcxx::string path, uvcxx::string new_path, int flags) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_copyfile, loop, req, path, new_path, flags);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> copyfile(uvcxx::string path, uvcxx::string new_path, int flags) {
            return copyfile(default_loop(), {}, path, new_path, flags);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> copyfile(const loop_t &loop, uvcxx::string path, uvcxx::string new_path, int flags) {
            return copyfile(loop, {}, path, new_path, flags);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> copyfile(const fs_t &req, uvcxx::string path, uvcxx::string new_path, int flags) {
            return copyfile(default_loop(), req, path, new_path, flags);
        }

#endif

        inline int sendfile(std::nullptr_t, fs_t &req,
                            uv_file out_fd, uv_file in_fd, int64_t in_offset, size_t length,
                            std::nullptr_t) {
            UVCXX_PROXY(uv_fs_sendfile(nullptr, req, out_fd, in_fd, in_offset, length, nullptr));
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<ssize_t> sendfile(
                const loop_t &loop, const fs_t &req,
                uv_file out_fd, uv_file in_fd, int64_t in_offset, size_t length) {
            return inner::invoker<ssize_t>()(
                    [](inner::raw_req_t *req) { return (ssize_t) req->result; },
                    uv_fs_sendfile, loop, req, out_fd, in_fd, in_offset, length);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<ssize_t> sendfile(
                uv_file out_fd, uv_file in_fd, int64_t in_offset, size_t length) {
            return sendfile(default_loop(), {}, out_fd, in_fd, in_offset, length);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<ssize_t> sendfile(
                const loop_t &loop, uv_file out_fd, uv_file in_fd, int64_t in_offset, size_t length) {
            return sendfile(loop, {}, out_fd, in_fd, in_offset, length);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<ssize_t> sendfile(
                const fs_t &req, uv_file out_fd, uv_file in_fd, int64_t in_offset, size_t length) {
            return sendfile(default_loop(), req, out_fd, in_fd, in_offset, length);
        }

        inline int access(std::nullptr_t, fs_t &req,
                          uvcxx::string path, int mode,
                          std::nullptr_t) {
            UVCXX_PROXY(uv_fs_access(nullptr, req, path, mode, nullptr));
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> access(const loop_t &loop, const fs_t &req, uvcxx::string path, int mode) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_access, loop, req, path, mode);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> access(uvcxx::string path, int mode) {
            return access(default_loop(), {}, path, mode);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> access(const loop_t &loop, uvcxx::string path, int mode) {
            return access(loop, {}, path, mode);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> access(const fs_t &req, uvcxx::string path, int mode) {
            return access(default_loop(), req, path, mode);
        }

        inline int chmod(std::nullptr_t, fs_t &req,
                         uvcxx::string path, int mode,
                         std::nullptr_t) {
            UVCXX_PROXY(uv_fs_chmod(nullptr, req, path, mode, nullptr));
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> chmod(const loop_t &loop, const fs_t &req, uvcxx::string path, int mode) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_chmod, loop, req, path, mode);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> chmod(uvcxx::string path, int mode) {
            return chmod(default_loop(), {}, path, mode);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> chmod(const loop_t &loop, uvcxx::string path, int mode) {
            return chmod(loop, {}, path, mode);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> chmod(const fs_t &req, uvcxx::string path, int mode) {
            return chmod(default_loop(), req, path, mode);
        }

        inline int fchmod(std::nullptr_t, fs_t &req,
                          uv_file file, int mode,
                          std::nullptr_t) {
            UVCXX_PROXY(uv_fs_fchmod(nullptr, req, file, mode, nullptr));
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> fchmod(const loop_t &loop, const fs_t &req, uv_file file, int mode) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_fchmod, loop, req, file, mode);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> fchmod(uv_file file, int mode) {
            return fchmod(default_loop(), {}, file, mode);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> fchmod(const loop_t &loop, uv_file file, int mode) {
            return fchmod(loop, {}, file, mode);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> fchmod(const fs_t &req, uv_file file, int mode) {
            return fchmod(default_loop(), req, file, mode);
        }

        inline int utime(std::nullptr_t, fs_t &req,
                         uvcxx::string path, double atime, double mtime,
                         std::nullptr_t) {
            UVCXX_PROXY(uv_fs_utime(nullptr, req, path, atime, mtime, nullptr));
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> utime(
                const loop_t &loop, const fs_t &req, uvcxx::string path, double atime, double mtime) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_utime, loop, req, path, atime, mtime);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> utime(uvcxx::string path, double atime, double mtime) {
            return utime(default_loop(), {}, path, atime, mtime);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> utime(const loop_t &loop, uvcxx::string path, double atime, double mtime) {
            return utime(loop, {}, path, atime, mtime);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> utime(const fs_t &req, uvcxx::string path, double atime, double mtime) {
            return utime(default_loop(), req, path, atime, mtime);
        }

        inline int futime(std::nullptr_t, fs_t &req,
                          uv_file file, double atime, double mtime,
                          std::nullptr_t) {
            UVCXX_PROXY(uv_fs_futime(nullptr, req, file, atime, mtime, nullptr));
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> futime(
                const loop_t &loop, const fs_t &req, uv_file file, double atime, double mtime) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_futime, loop, req, file, atime, mtime);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> futime(uv_file file, double atime, double mtime) {
            return futime(default_loop(), {}, file, atime, mtime);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> futime(const loop_t &loop, uv_file file, double atime, double mtime) {
            return futime(loop, {}, file, atime, mtime);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> futime(const fs_t &req, uv_file file, double atime, double mtime) {
            return futime(default_loop(), req, file, atime, mtime);
        }

#if UVCXX_SATISFY_VERSION(1, 36, 0)

        inline int lutime(std::nullptr_t, fs_t &req,
                          uvcxx::string path, double atime, double mtime,
                          std::nullptr_t) {
            UVCXX_PROXY(uv_fs_lutime(nullptr, req, path, atime, mtime, nullptr));
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> lutime(
                const loop_t &loop, const fs_t &req, uvcxx::string path, double atime, double mtime) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_lutime, loop, req, path, atime, mtime);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> lutime(uvcxx::string path, double atime, double mtime) {
            return lutime(default_loop(), {}, path, atime, mtime);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> lutime(const loop_t &loop, uvcxx::string path, double atime, double mtime) {
            return lutime(loop, {}, path, atime, mtime);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> lutime(const fs_t &req, uvcxx::string path, double atime, double mtime) {
            return lutime(default_loop(), req, path, atime, mtime);
        }

#endif

        inline int link(std::nullptr_t, fs_t &req,
                        uvcxx::string path, uvcxx::string new_path,
                        std::nullptr_t) {
            UVCXX_PROXY(uv_fs_link(nullptr, req, path, new_path, nullptr));
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> link(
                const loop_t &loop, const fs_t &req, uvcxx::string path, uvcxx::string new_path) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_link, loop, req, path, new_path);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> link(uvcxx::string path, uvcxx::string new_path) {
            return link(default_loop(), {}, path, new_path);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> link(const loop_t &loop, uvcxx::string path, uvcxx::string new_path) {
            return link(loop, {}, path, new_path);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> link(const fs_t &req, uvcxx::string path, uvcxx::string new_path) {
            return link(default_loop(), req, path, new_path);
        }

        inline int symlink(std::nullptr_t, fs_t &req,
                           uvcxx::string path, uvcxx::string new_path, int flags,
                           std::nullptr_t) {
            UVCXX_PROXY(uv_fs_symlink(nullptr, req, path, new_path, flags, nullptr));
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> symlink(
                const loop_t &loop, const fs_t &req, uvcxx::string path, uvcxx::string new_path, int flags) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_symlink, loop, req, path, new_path, flags);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> symlink(uvcxx::string path, uvcxx::string new_path, int flags) {
            return symlink(default_loop(), {}, path, new_path, flags);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> symlink(const loop_t &loop, uvcxx::string path, uvcxx::string new_path, int flags) {
            return symlink(loop, {}, path, new_path, flags);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> symlink(const fs_t &req, uvcxx::string path, uvcxx::string new_path, int flags) {
            return symlink(default_loop(), req, path, new_path, flags);
        }

        inline int readlink(std::nullptr_t, fs_t &req,
                            uvcxx::string path,
                            std::nullptr_t) {
            UVCXX_PROXY(uv_fs_readlink(nullptr, req, path, nullptr));
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<const char *> readlink(const loop_t &loop, const fs_t &req, uvcxx::string path) {
            return inner::invoker<const char *>()(
                    [](inner::raw_req_t *req) { return (const char *) req->ptr; },
                    uv_fs_readlink, loop, req, path);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<const char *> readlink(uvcxx::string path) {
            return readlink(default_loop(), {}, path);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<const char *> readlink(const loop_t &loop, uvcxx::string path) {
            return readlink(loop, {}, path);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<const char *> readlink(const fs_t &req, uvcxx::string path) {
            return readlink(default_loop(), req, path);
        }

#if UVCXX_SATISFY_VERSION(1, 8, 0)

        inline int realpath(std::nullptr_t, fs_t &req,
                            uvcxx::string path,
                            std::nullptr_t) {
            UVCXX_PROXY(uv_fs_realpath(nullptr, req, path, nullptr));
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<const char *> realpath(const loop_t &loop, const fs_t &req, uvcxx::string path) {
            return inner::invoker<const char *>()(
                    [](inner::raw_req_t *req) { return (const char *) req->ptr; },
                    uv_fs_realpath, loop, req, path);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<const char *> realpath(uvcxx::string path) {
            return realpath(default_loop(), {}, path);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<const char *> realpath(const loop_t &loop, uvcxx::string path) {
            return realpath(loop, {}, path);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<const char *> realpath(const fs_t &req, uvcxx::string path) {
            return realpath(default_loop(), req, path);
        }

#endif

        inline int chown(std::nullptr_t, fs_t &req,
                         uvcxx::string path, uv_uid_t uid, uv_gid_t gid,
                         std::nullptr_t) {
            UVCXX_PROXY(uv_fs_chown(nullptr, req, path, uid, gid, nullptr));
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> chown(
                const loop_t &loop, const fs_t &req, uvcxx::string path, uv_uid_t uid, uv_gid_t gid) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_chown, loop, req, path, uid, gid);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> chown(uvcxx::string path, uv_uid_t uid, uv_gid_t gid) {
            return chown(default_loop(), {}, path, uid, gid);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> chown(const loop_t &loop, uvcxx::string path, uv_uid_t uid, uv_gid_t gid) {
            return chown(loop, {}, path, uid, gid);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> chown(const fs_t &req, uvcxx::string path, uv_uid_t uid, uv_gid_t gid) {
            return chown(default_loop(), req, path, uid, gid);
        }

        inline int fchown(std::nullptr_t, fs_t &req,
                          uv_file file, uv_uid_t uid, uv_gid_t gid,
                          std::nullptr_t) {
            UVCXX_PROXY(uv_fs_fchown(nullptr, req, file, uid, gid, nullptr));
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> fchown(
                const loop_t &loop, const fs_t &req, uv_file file, uv_uid_t uid, uv_gid_t gid) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_fchown, loop, req, file, uid, gid);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> fchown(uv_file file, uv_uid_t uid, uv_gid_t gid) {
            return fchown(default_loop(), {}, file, uid, gid);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> fchown(const loop_t &loop, uv_file file, uv_uid_t uid, uv_gid_t gid) {
            return fchown(loop, {}, file, uid, gid);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> fchown(const fs_t &req, uv_file file, uv_uid_t uid, uv_gid_t gid) {
            return fchown(default_loop(), req, file, uid, gid);
        }

        inline int lchown(std::nullptr_t, fs_t &req,
                          uvcxx::string path, uv_uid_t uid, uv_gid_t gid,
                          std::nullptr_t) {
            UVCXX_PROXY(uv_fs_lchown(nullptr, req, path, uid, gid, nullptr));
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> lchown(
                const loop_t &loop, const fs_t &req, uvcxx::string path, uv_uid_t uid, uv_gid_t gid) {
            return inner::invoker<int>()(
                    [](inner::raw_req_t *req) { return (int) req->result; },
                    uv_fs_lchown, loop, req, path, uid, gid);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> lchown(uvcxx::string path, uv_uid_t uid, uv_gid_t gid) {
            return lchown(default_loop(), {}, path, uid, gid);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> lchown(const loop_t &loop, uvcxx::string path, uv_uid_t uid, uv_gid_t gid) {
            return lchown(loop, {}, path, uid, gid);
        }

        UVCXX_NODISCARD
        inline uvcxx::promise<int> lchown(const fs_t &req, uvcxx::string path, uv_uid_t uid, uv_gid_t gid) {
            return lchown(default_loop(), req, path, uid, gid);
        }
    }

#if UVCXX_SATISFY_VERSION(1, 12, 0)

    inline uv_os_fd_t get_osfhandle(int fd) { return uv_get_osfhandle(fd); }

#endif
#if UVCXX_SATISFY_VERSION(1, 23, 0)

    inline int open_osfhandle(uv_os_fd_t os_fd) { return uv_open_osfhandle(os_fd); }

#endif
}

#endif //LIBUVCXX_FS_H
