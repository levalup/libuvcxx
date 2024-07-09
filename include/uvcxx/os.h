//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_OS_H
#define LIBUVCXX_OS_H

#include <functional>

#include <uv.h>

#include "cxx/except.h"
#include "cxx/string.h"
#include "cxx/version.h"
#include "cxx/wrapper.h"
#include "inner/base.h"

namespace uv {
    namespace os {
#if UVCXX_SATISFY_VERSION(1, 18, 0)

        inline uv_pid_t getpid() {
            return uv_os_getpid();
        }

#endif
#if UVCXX_SATISFY_VERSION(1, 16, 0)

        inline uv_pid_t getppid() {
            return uv_os_getppid();
        }

#endif
#if UVCXX_SATISFY_VERSION(1, 6, 0)

        inline int homedir(char *buffer, size_t *size) {
            return uv_os_homedir(buffer, size);
        }

        inline std::string homedir() {
            return uvcxx::get_string<UVCXX_MAX_PATH>(uv_os_homedir);
        }

#endif
#if UVCXX_SATISFY_VERSION(1, 9, 0)

        inline int tmpdir(char *buffer, size_t *size) {
            return uv_os_tmpdir(buffer, size);
        }

        inline std::string tmpdir() {
            return uvcxx::get_string<UVCXX_MAX_PATH>(uv_os_tmpdir);
        }

        inline int get_passwd(uv_passwd_t *pwd) {
            UVCXX_PROXY(uv_os_get_passwd(pwd));
        }

        inline void free_passwd(uv_passwd_t *pwd) {
            uv_os_free_passwd(pwd);
        }

        class passwd_t : public uvcxx::inherit_raw_base_t<uv_passwd_t> {
        public:
            using self = passwd_t;
            using supper = uvcxx::inherit_raw_base_t<uv_passwd_t>;

            passwd_t(const passwd_t &) = delete;

            passwd_t &operator=(const passwd_t &) = delete;

            passwd_t(passwd_t &&that) UVCXX_NOEXCEPT {
                std::memset(raw(), 0, sizeof(raw_t));
                (void) this->operator=(std::move(that));
            };

            passwd_t &operator=(passwd_t &&) UVCXX_NOEXCEPT = default;

            passwd_t(std::nullptr_t) {
                std::memset(raw(), 0, sizeof(raw_t));
            }

            passwd_t() {
                UVCXX_APPLY_STRICT(uv_os_get_passwd(*this), "failed to get_passwd");
            }

            /**
             * Require >= 1.45.0
             * @param uid
             */
            explicit passwd_t(uv_uid_t uid);

            ~passwd_t() {
                if (*this) uv_os_free_passwd(*this);
            }

            explicit operator bool() const { return bool(raw()->username); }
        };

        inline passwd_t get_passwd() {
            return passwd_t{};
        }

#endif
#if UVCXX_SATISFY_VERSION(1, 45, 0)

        inline int get_passwd2(uv_passwd_t *pwd, uv_uid_t uid) {
            UVCXX_PROXY(uv_os_get_passwd2(pwd, uid));
        }

        inline int get_passwd(uv_passwd_t *pwd, uv_uid_t uid) {
            return get_passwd2(pwd, uid);
        }

        inline passwd_t::passwd_t(uv_uid_t uid) {
            UVCXX_APPLY_STRICT(uv_os_get_passwd2(*this, uid), "failed to get_passwd2(", (ssize_t) uid, ")");
        }

        inline passwd_t get_passwd(uv_uid_t uid) {
            return passwd_t{uid};
        }

        inline int get_group(uv_group_t *grp, uv_uid_t gid) {
            UVCXX_PROXY(uv_os_get_group(grp, gid));
        }

        inline void free_group(uv_group_t *grp) {
            uv_os_free_group(grp);
        }

        class group_t : public uvcxx::inherit_raw_base_t<uv_group_t> {
        public:
            using self = group_t;
            using supper = uvcxx::inherit_raw_base_t<uv_group_t>;

            group_t(const group_t &) = delete;

            group_t &operator=(const group_t &) = delete;

            group_t(group_t &&that) UVCXX_NOEXCEPT {
                std::memset(raw(), 0, sizeof(raw_t));
                (void) this->operator=(std::move(that));
            };

            group_t &operator=(group_t &&) UVCXX_NOEXCEPT = default;

            group_t(std::nullptr_t) {
                std::memset(raw(), 0, sizeof(raw_t));
            }

            explicit group_t(uv_uid_t gid) {
                UVCXX_APPLY_STRICT(uv_os_get_group(*this, gid), "failed to get_group(", (ssize_t) gid, ")");
            }

            ~group_t() {
                if (*this) uv_os_free_group(*this);
            }

            explicit operator bool() const { return bool(raw()->members); }
        };

        inline group_t get_group(uv_uid_t gid) {
            return group_t{gid};
        }

#endif
#if UVCXX_SATISFY_VERSION(1, 31, 0)

        inline int get_environ(uv_env_item_t **envitems, int *count) {
            UVCXX_PROXY(uv_os_environ(envitems, count));
        }

        inline void free_environ(uv_env_item_t *envitems, int count) {
            uv_os_free_environ(envitems, count);
        }

        class env_items_t : public uvcxx::base_t {
        public:
            using self = env_items_t;
            using raw_t = uv_env_item_t;

            env_items_t(const env_items_t &) = delete;

            env_items_t &operator=(const env_items_t &) = delete;

            env_items_t(env_items_t &&that) UVCXX_NOEXCEPT {
                (void) this->operator=(std::move(that));
            }

            env_items_t &operator=(env_items_t &&that) UVCXX_NOEXCEPT {
                std::swap(m_items, that.m_items);
                std::swap(m_count, that.m_count);
                return *this;
            }

            env_items_t() {
                UVCXX_APPLY_STRICT(uv_os_environ(&m_items, &m_count), "failed to retrieve environ");
            }

            ~env_items_t() {
                if (m_items) {
                    uv_os_free_environ(m_items, m_count);
                }
            }

            UVCXX_NODISCARD
            bool empty() const { return !m_count; }

            UVCXX_NODISCARD
            int size() const { return m_count; }

            UVCXX_NODISCARD
            const raw_t *begin() const { return m_items; }

            UVCXX_NODISCARD
            const raw_t *end() const { return m_items + m_count; }

        private:
            raw_t *m_items = nullptr;
            int m_count = 0;
        };

        UVCXX_NODISCARD
        inline env_items_t get_environ() {
            return {};
        }

#endif
#if !UVCXX_SATISFY_VERSION(1, 26, 0) && !defined(UV_MAXHOSTNAMESIZE)
#define UV_MAXHOSTNAMESIZE 257
#endif
#if UVCXX_SATISFY_VERSION(1, 12, 0)

        inline int getenv(uvcxx::string name, char *buffer, size_t *size) {
            return uv_os_getenv(name, buffer, size);
        }

        inline std::string getenv(uvcxx::string name) {
            const char *c_name = name;
            return uvcxx::get_string<UVCXX_MAX_PATH>([c_name](char *buffer, size_t *size) {
                return uv_os_getenv(c_name, buffer, size);
            });
        }

        inline int setenv(uvcxx::string name, uvcxx::string value) {
            UVCXX_PROXY(uv_os_setenv(name, value), "can not setenv['", name.c_str, "']='", value.c_str, "'");
        }

        inline int unsetenv(uvcxx::string name) {
            UVCXX_PROXY(uv_os_unsetenv(name), "can not unsetenv['", name.c_str, "']");
        }

        inline int gethostname(char *buffer, size_t *size) {
            return uv_os_gethostname(buffer, size);
        }

        inline std::string gethostname() {
            return uvcxx::get_string<UV_MAXHOSTNAMESIZE - 1>(uv_os_gethostname);
        }

#endif
#if UVCXX_SATISFY_VERSION(1, 23, 0)

        inline int getpriority(uv_pid_t pid, int *priority) {
            UVCXX_PROXY(uv_os_getpriority(pid, priority), "get priority of pid=", pid);
        }

        inline int setpriority(uv_pid_t pid, int priority) {
            UVCXX_PROXY(uv_os_setpriority(pid, priority), "set priority of pid=", pid);
        }

        inline int getpriority(uv_pid_t pid) {
            int priority = 0;
            (void) getpriority(pid, &priority);
            return priority;
        }

#endif
#if UVCXX_SATISFY_VERSION(1, 25, 0)

        inline int uname(uv_utsname_t *buffer) {
            UVCXX_PROXY(uv_os_uname(buffer), "failed retrieve system information");
        }

#endif
    }
}

#endif //LIBUVCXX_OS_H
