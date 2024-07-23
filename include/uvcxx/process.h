//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_PROCESS_H
#define LIBUVCXX_PROCESS_H

#include <cstring>
#include <sstream>

#include "cxx/string.h"
#include "handle.h"

namespace uv {
    class process_option_t : public uvcxx::extend_raw_base_t<uv_process_options_t> {
    public:
        using self = process_option_t;
        using supper = uvcxx::extend_raw_base_t<uv_process_options_t>;

        process_option_t(const process_option_t &) = delete;

        process_option_t &operator=(const process_option_t &) = delete;

        process_option_t(process_option_t &&that) UVCXX_NOEXCEPT {
            (void) operator=(std::move(that));
        }

        process_option_t &operator=(process_option_t &&that) UVCXX_NOEXCEPT {
            *(raw()) = *that.raw();
            m_cxx = std::move(that.m_cxx);
            m_cxx.dump(*this);
            return *this;
        }

        explicit process_option_t(const char *file) {
            std::memset(raw(), 0, sizeof(raw_t));
            raw()->file = file;
            m_cxx.dump_args(*this);
        }

        explicit process_option_t(std::string file) {
            m_cxx.file = std::move(file);
            std::memset(raw(), 0, sizeof(raw_t));
            m_cxx.dump_file(*this);
            m_cxx.dump_args(*this);
        }

        self &args(char **args) {
            raw()->args = args;
            return *this;
        }

        self &args(std::vector<std::string> args) {
            m_cxx.args = std::move(args);
            m_cxx.dump_args(*this);
            return *this;
        }

        self &args(std::initializer_list<std::string> args) {
            return this->args(std::vector<std::string>{args.begin(), args.end()});
        }

        self &env(char **env) {
            raw()->env = env;
            return *this;
        }

        self &env(std::vector<std::string> env) {
            m_cxx.env = std::move(env);
            m_cxx.dump_env(*this);
            return *this;
        }

        self &env(std::initializer_list<std::string> env) {
            return this->env(std::vector<std::string>{env.begin(), env.end()});
        }

        self &cwd(const char *cwd) {
            raw()->cwd = cwd;
            return *this;
        }

        self &cwd(std::string cwd) {
            m_cxx.cwd = std::move(cwd);
            return this->cwd(m_cxx.cwd.c_str());
        }

        self &flags(unsigned int flags) {
            raw()->flags |= flags;
            return *this;
        }

        self &stdio(uv_stdio_container_t *stdio, int stdio_count) {
            raw()->stdio = stdio;
            raw()->stdio_count = stdio_count;
            return *this;
        }

        self &stdio(std::vector<uv_stdio_container_t> stdio) {
            m_cxx.stdio = std::move(stdio);
            return this->stdio(m_cxx.stdio.data(), (int) m_cxx.stdio.size());
        }

        self &uid(uv_uid_t uid) {
            raw()->uid = uid;
            return *this;
        }

        self &gid(uv_gid_t gid) {
            raw()->gid = gid;
            return *this;
        }

        self &flag(unsigned int flag) {
            raw()->flags |= flag;
            return *this;
        }

        self &flag(std::nullptr_t) {
            raw()->flags = 0;
            return *this;
        }

        template<typename V, typename std::enable_if<std::is_convertible<std::ostream &,
                decltype(std::declval<std::ostream &>() << std::declval<const V &>())>::value, int>::type = 0>
        self &env(uvcxx::string_view key, const V &value) {
            std::ostringstream oss;
            oss << key << "=" << value;
            m_cxx.env.emplace_back(oss.str());
            m_cxx.dump_env(*this);
            return *this;
        }

        self &env(uvcxx::string_view key, uvcxx::string_view value) {
            std::ostringstream oss;
            oss << key << "=" << value;
            m_cxx.env.emplace_back(oss.str());
            m_cxx.dump_env(*this);
            return *this;
        }

        self &stdio(int i, uv_stdio_container_t *io) {
            auto size = size_t(i) + 1;
            if (size > m_cxx.stdio.size()) {
                uv_stdio_container_t ignore{};
                ignore.flags = UV_IGNORE;
                ignore.data.fd = -1;
                m_cxx.stdio.resize(size, ignore);
            }
            m_cxx.stdio[i] = *io;
            return this->stdio(m_cxx.stdio.data(), (int) m_cxx.stdio.size());
        }

        self &stdio(int i, uv_stdio_flags flags) {
            uv_stdio_container_t sc{};
            sc.flags = flags;
            return this->stdio(i, &sc);
        }

        self &stdio(int i, uv_stdio_flags flags, uv_stream_t *stream) {
            uv_stdio_container_t sc{};
            sc.flags = flags;
            sc.data.stream = stream;
            return this->stdio(i, &sc);
        }

        self &stdio(int i, uv_stdio_flags flags, int fd) {
            uv_stdio_container_t sc{};
            sc.flags = flags;
            sc.data.fd = fd;
            return this->stdio(i, &sc);
        }

    private:
        struct {
            std::string file;
            std::vector<std::string> args;
            std::vector<std::string> env;
            std::string cwd;
            std::vector<uv_stdio_container_t> stdio;

            mutable std::vector<const char *> c_args;
            mutable std::vector<const char *> c_env;

            void dump_file(uv_process_options_t *option) const {
                option->file = file.c_str();
            }

            void dump_args(uv_process_options_t *option) const {
                c_args.resize(args.size() + 1);
                for (size_t i = 0; i < args.size(); ++i) {
                    c_args[i] = args[i].c_str();
                }
                c_args.back() = nullptr;
                option->args = (char **) c_args.data();
            }

            void dump_env(uv_process_options_t *option) const {
                c_env.resize(env.size() + 1);
                for (size_t i = 0; i < env.size(); ++i) {
                    c_env[i] = env[i].c_str();
                }
                c_env.back() = nullptr;
                option->env = (char **) c_env.data();
            }

            void dump_cwd(uv_process_options_t *option) const {
                option->cwd = cwd.c_str();
            }

            void dump_stdio(uv_process_options_t *option) const {
                option->stdio = (uv_stdio_container_t *) stdio.data();
                option->stdio_count = (int) stdio.size();
            }

            void dump(uv_process_options_t *option) const {
                dump_file(option);
                dump_args(option);
                dump_env(option);
                dump_cwd(option);
                dump_stdio(option);
            }
        } m_cxx;
    };

    /**
     * After the process is successfully `spawn`, `close` will be automatically called when the process exits,
     *     so there is no need to explicitly call `close` externally.
     */
    class process_t : public inherit_handle_t<uv_process_t, handle_t> {
    public:
        using self = process_t;
        using supper = inherit_handle_t<uv_process_t, handle_t>;

        process_t() {
            set_data(new data_t(*this));
        }

        UVCXX_NODISCARD
        int pid() const {
            return raw<raw_t>()->pid;
        }

        UVCXX_NODISCARD
        uvcxx::promise<int64_t, int> spawn(const loop_t &loop, const uv_process_options_t *options) {
            auto fix_options = *options;
            fix_options.exit_cb = raw_exit_callback;

            UVCXX_APPLY(uv_spawn(loop, *this, &fix_options), nullptr);

            _detach_();
            return get_data<data_t>()->exit_cb.promise();
        }

        UVCXX_NODISCARD
        uvcxx::promise<int64_t, int> spawn(const uv_process_options_t *options) {
            return this->spawn(default_loop(), options);
        }

        int kill(int signum) {
            UVCXX_PROXY(uv_process_kill(*this, signum));
        }

        UVCXX_NODISCARD
        uv_pid_t get_pid() const {
            // cover uv_process_get_pid
            return (uv_pid_t) raw<raw_t>()->pid;
        }

    private:
        static void raw_exit_callback(uv_process_t *handle, int64_t exit_status, int term_signal) {
            auto data = (data_t *) handle->data;
            if (!data_t::is_it(data)) return;

            data->exit_cb.resolve(exit_status, term_signal);

            data->close_for((uv_handle_t *) handle, [&](void (*cb)(uv_handle_t *)) {
                uv_close((uv_handle_t *) handle, cb);
            });
        }

        class data_t : public handle_t::data_t {
        public:
            uvcxx::promise_emitter<int64_t, int> exit_cb;

            explicit data_t(const handle_t &handle) : handle_t::data_t(handle) {
            }
        };
    };

    inline void disable_stdio_inheritance() { uv_disable_stdio_inheritance(); }

    inline int kill(int pid, int signum) {
        UVCXX_PROXY(uv_kill(pid, signum));
    }
}

#endif //LIBUVCXX_PROCESS_H
