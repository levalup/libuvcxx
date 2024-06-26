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
#include "cxx/utils.h"
#include "cxx/version.h"
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
            return uvcxx::try_get<UVCXX_MAX_PATH>(uv_os_homedir);
        }

#endif
#if UVCXX_SATISFY_VERSION(1, 9, 0)

        inline int tmpdir(char *buffer, size_t *size) {
            return uv_os_tmpdir(buffer, size);
        }

        inline std::string tmpdir() {
            return uvcxx::try_get<UVCXX_MAX_PATH>(uv_os_tmpdir);
        }

        inline int get_passwd(uv_passwd_t *pwd) {
            return uv_os_get_passwd(pwd);
        }

        inline void free_passwd(uv_passwd_t *pwd) {
            uv_os_free_passwd(pwd);
        }

        class passwd_t : public uvcxx::inherit_raw_base_t<uv_passwd_t> {
        public:
            passwd_t(const passwd_t &) = delete;

            passwd_t &operator=(const passwd_t &) = delete;

            passwd_t(passwd_t &&) = default;

            passwd_t &operator=(passwd_t &&) = default;

            passwd_t() {
                auto status = uv_os_get_passwd(this);
                if (status < 0) throw uvcxx::errcode(status, "failed to get_passwd");
            }

            /**
             * Require >= 1.45.0
             * @param uid
             */
            explicit passwd_t(uv_uid_t uid);

            ~passwd_t() {
                uv_os_free_passwd(this);
            }
        };

        inline passwd_t get_passwd() {
            return passwd_t{};
        }

#endif
#if UVCXX_SATISFY_VERSION(1, 45, 0)

        inline int get_passwd2(uv_passwd_t *pwd, uv_uid_t uid) {
            return uv_os_get_passwd2(pwd, uid);
        }

        inline int get_passwd(uv_passwd_t *pwd, uv_uid_t uid) {
            return get_passwd2(pwd, uid);
        }

        inline passwd_t::passwd_t(uv_uid_t uid) {
            auto status = uv_os_get_passwd2(this, uid);
            if (status < 0) throw uvcxx::errcode(status, "failed to get_passwd2");
        }

        inline passwd_t get_passwd(uv_uid_t uid) {
            return passwd_t{uid};
        }

        inline int get_group(uv_group_t *grp, uv_uid_t gid) {
            return uv_os_get_group(grp, gid);
        }

        inline void free_group(uv_group_t *grp) {
            uv_os_free_group(grp);
        }

        class group_t : public uvcxx::inherit_raw_base_t<uv_group_t> {
        public:
            group_t(const group_t &) = delete;

            group_t &operator=(const group_t &) = delete;

            group_t(group_t &&) = default;

            group_t &operator=(group_t &&) = default;

            explicit group_t(uv_uid_t gid) {
                auto status = uv_os_get_group(this, gid);
                if (status < 0) throw uvcxx::errcode(status, "failed to get_group");
            }

            ~group_t() {
                uv_os_free_group(this);
            }
        };

        inline group_t get_group(uv_uid_t gid) {
            return group_t{gid};
        }

#endif
#if UVCXX_SATISFY_VERSION(1, 31, 0)

        inline int get_environ(uv_env_item_t **envitems, int *count) {
            return uv_os_environ(envitems, count);
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

            env_items_t(env_items_t &&) = default;

            env_items_t &operator=(env_items_t &&) = default;

            env_items_t() {
                auto status = uv_os_environ(&m_items, &m_count);
                if (status < 0) UVCXX_THROW(status, "failed to retrieve environ");
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
            return uvcxx::try_get<UVCXX_MAX_PATH>([c_name](char *buffer, size_t *size) {
                return uv_os_getenv(c_name, buffer, size);
            });
        }

        int setenv(uvcxx::string name, uvcxx::string value) {
            auto status = uv_os_setenv(name, value);
            if (status < 0)
                UVCXX_THROW_OR_RETURN(
                        status, status, "can not setenv['", name.c_str, "']='", value.c_str, "'");
            return status;
        }

        int unsetenv(uvcxx::string name) {
            auto status = uv_os_unsetenv(name);
            if (status < 0)
                UVCXX_THROW_OR_RETURN(
                        status, status, "can not unsetenv['", name.c_str, "']");
            return status;
        }

        inline int gethostname(char *buffer, size_t *size) {
            return uv_os_gethostname(buffer, size);
        }

        inline std::string gethostname() {
            return uvcxx::try_get<UV_MAXHOSTNAMESIZE - 1>(uv_os_gethostname);
        }

#endif
#if UVCXX_SATISFY_VERSION(1, 23, 0)

        inline int getpriority(uv_pid_t pid, int *priority) {
            auto status = uv_os_getpriority(pid, priority);
            if (status < 0) UVCXX_THROW_OR_RETURN(status, status, "get priority of pid=", pid);
            return status;
        }

        inline int setpriority(uv_pid_t pid, int priority) {
            auto status = uv_os_setpriority(pid, priority);
            if (status < 0) UVCXX_THROW_OR_RETURN(status, status, "set priority of pid=", pid);
            return status;
        }

        inline int getpriority(uv_pid_t pid) {
            int priority = 0;
            auto status = uv_os_getpriority(pid, &priority);
            if (status < 0) UVCXX_THROW_OR_RETURN(status, priority, "set priority of pid=", pid);
            return priority;
        }

#endif
#if UVCXX_SATISFY_VERSION(1, 25, 0)

        inline int uname(uv_utsname_t *buffer) {
            auto status = uv_os_uname(buffer);
            if (status < 0) UVCXX_THROW_OR_RETURN(status, status, "failed retrieve system information");
            return status;
        }

#endif
    }
}

#endif //LIBUVCXX_OS_H
