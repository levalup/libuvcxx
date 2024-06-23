//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_PROCESS_H
#define LIBUVCXX_PROCESS_H

#include <cstring>
#include <sstream>

#include "handle.h"

namespace uv {
    class process_option_t : public uvcxx::extend_raw_base_t<uv_process_options_t> {
    public:
        using self = process_option_t;
        using supper = uvcxx::extend_raw_base_t<uv_process_options_t>;

        process_option_t(const process_option_t &) = delete;

        process_option_t &operator=(const process_option_t &) = delete;

        explicit process_option_t(const char *file) {
            std::memset(raw(), 0, sizeof(raw_t));
            raw()->file = file;

            m_c_args.resize(1, nullptr);
            raw()->args = (char **) m_c_args.data();
        }

        explicit process_option_t(std::string file)
                : m_file(std::move(file)) {
            std::memset(raw(), 0, sizeof(raw_t));
            raw()->file = m_file.c_str();

            m_c_args.resize(1, nullptr);
            raw()->args = (char **) m_c_args.data();
        }

        self &args(char **args) {
            raw()->args = args;
            return *this;
        }

        self &args(std::vector<std::string> args) {
            m_args = std::move(args);
            m_c_args.resize(m_args.size() + 1);

            for (size_t i = 0; i < m_args.size(); ++i) {
                m_c_args[i] = m_args[i].c_str();
            }
            m_c_args.back() = nullptr;

            return this->args((char **) m_c_args.data());
        }

        self &args(const std::initializer_list<std::string> &args) {
            return this->args(std::vector<std::string>{args.begin(), args.end()});
        }

        self &env(char **env) {
            raw()->env = env;
            return *this;
        }

        self &env(std::vector<std::string> env) {
            m_env = std::move(env);
            m_c_env.resize(m_env.size() + 1);

            for (size_t i = 0; i < m_env.size(); ++i) {
                m_c_env[i] = m_env[i].c_str();
            }
            m_c_env.back() = nullptr;

            return this->env((char **) m_c_env.data());
        }

        self &env(const std::initializer_list<std::string> &env) {
            return this->env(std::vector<std::string>{env.begin(), env.end()});
        }

        self &cwd(const char *cwd) {
            raw()->cwd = cwd;
            return *this;
        }

        self &cwd(std::string cwd) {
            m_cwd = std::move(cwd);
            return this->cwd(m_cwd.c_str());
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
            m_stdio = std::move(stdio);
            return this->stdio(m_stdio.data(), (int) m_stdio.size());
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

        template<typename V, typename std::enable_if_t<std::is_convertible_v<std::ostream &,
                decltype(std::declval<std::ostream &>() << std::declval<const V &>())>, int> = 0>
        self &env(const std::string_view &key, const V &value) {
            std::ostringstream oss;
            oss << key << "=" << value;
            m_env.emplace_back(oss.str());
            m_c_env.resize(m_env.size() + 1);
            for (size_t i = 0; i < m_env.size(); ++i) {
                m_c_env[i] = m_env[i].c_str();
            }
            m_c_env.back() = nullptr;
            return this->env((char **) m_c_env.data());
        }

        self &env(const std::string_view &key, const std::string_view &value) {
            std::ostringstream oss;
            oss << key << "=" << value;
            m_env.emplace_back(oss.str());
            m_c_env.resize(m_env.size() + 1);
            for (size_t i = 0; i < m_env.size(); ++i) {
                m_c_env[i] = m_env[i].c_str();
            }
            m_c_env.back() = nullptr;
            return this->env((char **) m_c_env.data());
        }

        self &stdio(int i, uv_stdio_container_t *io) {
            auto size = size_t(i) + 1;
            if (size > m_stdio.size()) {
                uv_stdio_container_t ignore{};
                ignore.flags = UV_IGNORE;
                m_stdio.resize(size, ignore);
            }
            m_stdio[i] = *io;
            return this->stdio(m_stdio.data(), (int) m_stdio.size());
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
        std::string m_file;
        std::vector<std::string> m_args;
        std::vector<const char *> m_c_args;
        std::vector<std::string> m_env;
        std::vector<const char *> m_c_env;
        std::string m_cwd;
        std::vector<uv_stdio_container_t> m_stdio;
    };

    class process_t;

    namespace inner {
        /**
         * inner process class
         * add support for join, exit callback, state grab and so on.
         */
        class process_t : public uv_process_t {
        public:
            using self = process_t;
            using supper = uv_process_t;

            process_t() : supper() {
                m_option.exit_cb = raw_exit_callback;
            }

            uvcxx::promise<int64_t, int> spawn(const loop_t &loop) {
                auto err = uv_spawn(loop, this, &m_option);
                if (err < 0) UVCXX_THROW_OR_RETURN(err, nullptr);

                auto data = (data_t * )
                this->data;
                return data->exit_cb.promise();
            }

        private:
            uv_process_options_t m_option{};

        private:
            static void raw_exit_callback(uv_process_t *handle, int64_t exit_status, int term_signal) {
                auto data = (data_t *) handle->data;
                if (!data_t::is_it(data)) return;
                data->exit_cb.resolve(exit_status, term_signal);
            }

            class data_t : public handle_t::data_t {
            public:
                uvcxx::promise_emitter<int64_t, int> exit_cb;

                explicit data_t(const handle_t &handle) : handle_t::data_t(handle) {
                }
            };

            friend class ::uv::process_t;
        };
    }

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
            _attach_data_();
        }

        [[nodiscard]]
        int pid() const {
            return raw<raw_t>()->pid;
        }

        uvcxx::promise<int64_t, int> spawn(const loop_t &loop, const uv_process_options_t *options) {
            auto fix_options = *options;
            fix_options.exit_cb = raw_exit_callback;

            auto status = uv_spawn(loop, *this, &fix_options);
            if (status < 0) UVCXX_THROW_OR_RETURN(status, nullptr);

            _detach_();
            return get_data<data_t>()->exit_cb.promise();
        }

        uvcxx::promise<int64_t, int> spawn(const uv_process_options_t *options) {
            return this->spawn(default_loop(), options);
        }

        int kill(int signum) {
            auto status = uv_process_kill(*this, signum);
            if (status < 0) UVCXX_THROW_OR_RETURN(status, status);
            return status;
        }

        [[nodiscard]]
        uv_pid_t get_pid() const {
            // cover uv_process_get_pid
            return (uv_pid_t) raw<raw_t>()->pid;
        }

    private:
        static void raw_exit_callback(uv_process_t *handle, int64_t exit_status, int term_signal) {
            auto data = (data_t *) handle->data;
            if (!data_t::is_it(data)) return;

            data->exit_cb.resolve(exit_status, term_signal);

            data->close_for([&](void (*cb)(uv_handle_t *)) {
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
        auto status = uv_kill(pid, signum);
        if (status < 0) UVCXX_THROW_OR_RETURN(status, status);
        return status;
    }
}

#endif //LIBUVCXX_PROCESS_H
