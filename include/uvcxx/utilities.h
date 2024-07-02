//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_UTILITIES_H
#define LIBUVCXX_UTILITIES_H

#include "cxx/except.h"
#include "cxx/string.h"
#include "cxx/version.h"
#include "cxx/wrapper.h"
#include "inner/base.h"

#if !UVCXX_SATISFY_VERSION(1, 34, 0)

#include <chrono>
#include <thread>

#endif

namespace uv {
    inline uv_handle_type guess_handle(uv_file file) {
        return uv_guess_handle(file);
    }

#if UVCXX_SATISFY_VERSION(1, 6, 0)

    inline int replace_allocator(
            uv_malloc_func malloc_func,
            uv_realloc_func realloc_func,
            uv_calloc_func calloc_func,
            uv_free_func free_func) {
        UVCXX_PROXY(uv_replace_allocator(malloc_func, realloc_func, calloc_func, free_func));
    }

#endif
#if UVCXX_SATISFY_VERSION(1, 38, 0)

    inline void library_shutdown() {
        uv_library_shutdown();
    }

#endif

    inline char **setup_args(int argc, char **argv) {
        return uv_setup_args(argc, argv);
    }

    inline int get_process_title(char *buffer, size_t size) {
        return uv_get_process_title(buffer, size);
    }

    inline std::string get_process_title() {
        return uvcxx::get_string<256>(uv_get_process_title);
    }

    inline int set_process_title(uvcxx::string title) {
        UVCXX_PROXY(uv_set_process_title(title));
    }

    inline int resident_set_memory(size_t *rss) {
        UVCXX_PROXY(uv_resident_set_memory(rss));
    }

    inline int uptime(double *uptime) {
        UVCXX_PROXY(uv_uptime(uptime));
    }

    inline int getrusage(uv_rusage_t *rusage) {
        UVCXX_PROXY(uv_getrusage(rusage));
    }

#if UVCXX_SATISFY_VERSION(1, 44, 0)

    inline unsigned int available_parallelism() {
        return uv_available_parallelism();
    }

#endif

    inline int cpu_info(uv_cpu_info_t **cpu_infos, int *count) {
        UVCXX_PROXY(uv_cpu_info(cpu_infos, count));
    }

    inline void free_cpu_info(uv_cpu_info_t *cpu_infos, int count) {
        uv_free_cpu_info(cpu_infos, count);
    }

    class cpu_infos_t : public uvcxx::base_t {
    public:
        using self = cpu_infos_t;
        using raw_t = uv_cpu_info_t;

        cpu_infos_t(const cpu_infos_t &) = delete;

        cpu_infos_t &operator=(const cpu_infos_t &) = delete;

        cpu_infos_t(cpu_infos_t &&that) UVCXX_NOEXCEPT {
            (void) this->operator=(std::move(that));
        }

        cpu_infos_t &operator=(cpu_infos_t &&that) UVCXX_NOEXCEPT {
            std::swap(m_infos, that.m_infos);
            std::swap(m_count, that.m_count);
            return *this;
        }

        cpu_infos_t() {
            UVCXX_APPLY_STRICT(uv_cpu_info(&m_infos, &m_count), "failed to retrieve cpu info");
        }

        ~cpu_infos_t() {
            if (m_infos) {
                uv_free_cpu_info(m_infos, m_count);
            }
        }

        UVCXX_NODISCARD
        bool empty() const { return !m_count; }

        UVCXX_NODISCARD
        int size() const { return m_count; }

        UVCXX_NODISCARD
        const raw_t *begin() const { return m_infos; }

        UVCXX_NODISCARD
        const raw_t *end() const { return m_infos + m_count; }

    private:
        raw_t *m_infos = nullptr;
        int m_count = 0;
    };

    UVCXX_NODISCARD
    inline cpu_infos_t cpu_info() {
        return {};
    }

#if UVCXX_SATISFY_VERSION(1, 45, 0)

    inline int cpumask_size() {
        UVCXX_PROXY(uv_cpumask_size());
    }

#endif

    inline int interface_addresses(uv_interface_address_t **addresses, int *count) {
        UVCXX_PROXY(uv_interface_addresses(addresses, count));
    }

    inline void free_interface_addresses(uv_interface_address_t *addresses, int count) {
        uv_free_interface_addresses(addresses, count);
    }

    class interface_addresses_t : public uvcxx::base_t {
    public:
        using self = interface_addresses_t;
        using raw_t = uv_interface_address_t;

        interface_addresses_t(const interface_addresses_t &) = delete;

        interface_addresses_t &operator=(const interface_addresses_t &) = delete;

        interface_addresses_t(interface_addresses_t &&that) UVCXX_NOEXCEPT {
            (void) this->operator=(std::move(that));
        }

        interface_addresses_t &operator=(interface_addresses_t &&that) UVCXX_NOEXCEPT {
            std::swap(m_addresses, that.m_addresses);
            std::swap(m_count, that.m_count);
            return *this;
        }

        interface_addresses_t() {
            UVCXX_APPLY_STRICT(uv_interface_addresses(&m_addresses, &m_count), "failed to retrieve network addresses");
        }

        ~interface_addresses_t() {
            if (m_addresses) {
                uv_free_interface_addresses(m_addresses, m_count);
            }
        }

        UVCXX_NODISCARD
        bool empty() const { return !m_count; }

        UVCXX_NODISCARD
        int size() const { return m_count; }

        UVCXX_NODISCARD
        const raw_t *begin() const { return m_addresses; }

        UVCXX_NODISCARD
        const raw_t *end() const { return m_addresses + m_count; }

    private:
        raw_t *m_addresses = nullptr;
        int m_count = 0;
    };

    UVCXX_NODISCARD
    inline interface_addresses_t interface_addresses() {
        return {};
    }

    inline void loadavg(double avg[3]) {
        uv_loadavg(avg);
    }

    inline int ip4_addr(uvcxx::string ip, int port, sockaddr_in *addr) {
        UVCXX_PROXY(uv_ip4_addr(ip, port, addr));
    }

    inline int ip6_addr(uvcxx::string ip, int port, sockaddr_in6 *addr) {
        UVCXX_PROXY(uv_ip6_addr(ip, port, addr));
    }

    inline int ip4_name(const sockaddr_in *src, char *dst, size_t size) {
        UVCXX_PROXY(uv_ip4_name(src, dst, size));
    }

    inline std::string ip4_name(const sockaddr_in *src) {
        char dst[UV_IF_NAMESIZE] = {0};
        (void) ip4_name(src, dst, sizeof(dst));
        return dst;
    }

    inline int ip6_name(const sockaddr_in6 *src, char *dst, size_t size) {
        UVCXX_PROXY(uv_ip6_name(src, dst, size));
    }

    inline std::string ip6_name(const sockaddr_in6 *src) {
        char dst[UV_IF_NAMESIZE] = {0};
        (void) ip6_name(src, dst, sizeof(dst));
        return dst;
    }

#if UVCXX_SATISFY_VERSION(1, 43, 0)

    inline int ip_name(const sockaddr *src, char *dst, size_t size) {
        UVCXX_PROXY(uv_ip_name(src, dst, size));
    }

    inline std::string ip_name(const sockaddr *src) {
        char dst[UV_IF_NAMESIZE] = {0};
        (void) ip_name(src, dst, sizeof(dst));
        return dst;
    }

#endif

    inline int inet_ntop(int af, uvcxx::string src, char *dst, size_t size) {
        UVCXX_PROXY(uv_inet_ntop(af, src, dst, size));
    }

    inline int inet_pton(int af, uvcxx::string src, char *dst) {
        UVCXX_PROXY(uv_inet_pton(af, src, dst));
    }

#if !UVCXX_SATISFY_VERSION(1, 16, 0) && !defined(UV_IF_NAMESIZE)
#define UV_IF_NAMESIZE (16 + 1)
#endif

#if UVCXX_SATISFY_VERSION(1, 16, 0)

    inline int if_indextoname(unsigned int ifindex, char *buffer, size_t *size) {
        return uv_if_indextoname(ifindex, buffer, size);
    }

    inline std::string if_indextoname(unsigned int ifindex) {
        return uvcxx::get_string<UV_IF_NAMESIZE - 1>([ifindex](char *buffer, size_t *size) {
            return uv_if_indextoname(ifindex, buffer, size);
        });
    }

    inline int if_indextoiid(unsigned int ifindex, char *buffer, size_t *size) {
        return uv_if_indextoiid(ifindex, buffer, size);
    }

    inline std::string if_indextoiid(unsigned int ifindex) {
        return uvcxx::get_string<UV_IF_NAMESIZE - 1>([ifindex](char *buffer, size_t *size) {
            return uv_if_indextoiid(ifindex, buffer, size);
        });
    }

#endif

    inline int exepath(char *buffer, size_t *size) {
        return uv_exepath(buffer, size);
    }

    inline std::string exepath() {
        return uvcxx::get_string<UVCXX_MAX_PATH>(uv_exepath);
    }

    inline int cwd(char *buffer, size_t *size) {
        return uv_cwd(buffer, size);
    }

    inline std::string cwd() {
        return uvcxx::get_string<UVCXX_MAX_PATH>(uv_cwd);
    }

    inline int chdir(uvcxx::string dir) {
        UVCXX_PROXY(uv_chdir(dir));
    }

    inline uint64_t get_free_memory() {
        return uv_get_free_memory();
    }

    inline uint64_t get_total_memory() {
        return uv_get_total_memory();
    }

#if UVCXX_SATISFY_VERSION(1, 29, 0)

    inline uint64_t get_constrained_memor() {
        return uv_get_constrained_memory();
    }

#endif
#if UVCXX_SATISFY_VERSION(1, 45, 0)

    inline uint64_t get_available_memory() {
        return uv_get_available_memory();
    }

#endif

    inline uint64_t hrtime() {
        return uv_hrtime();
    }

#if UVCXX_SATISFY_VERSION(1, 45, 0)

    inline int clock_gettime(uv_clock_id clock_id, uv_timespec64_t *ts) {
        return uv_clock_gettime(clock_id, ts);
    }

    inline uv_timespec64_t clock_gettime(uv_clock_id clock_id) {
        uv_timespec64_t ts{};
        std::memset(&ts, 0, sizeof(ts));
        (void) clock_gettime(clock_id, &ts);
        return ts;
    }

#endif
#if UVCXX_SATISFY_VERSION(1, 28, 0)

    inline int gettimeofday(uv_timeval64_t *tv) {
        return uv_gettimeofday(tv);
    }

    inline uv_timeval64_t gettimeofday() {
        uv_timeval64_t tv{};
        std::memset(&tv, 0, sizeof(tv));
        (void) gettimeofday(&tv);
        return tv;
    }

#endif

#if UVCXX_SATISFY_VERSION(1, 34, 0)

    inline void sleep(unsigned int msec) { return uv_sleep(msec); }

#else

    inline void sleep(unsigned int msec) {
        std::this_thread::sleep_for(std::chrono::milliseconds(msec));
    }

#endif
#if UVCXX_SATISFY_VERSION(1, 47, 0)

    inline size_t utf16_length_as_wtf8(const uint16_t *utf16, ssize_t utf16_len) {
        return uv_utf16_length_as_wtf8(utf16, utf16_len);
    }

    inline int utf16_to_wtf8(const uint16_t *utf16, ssize_t utf16_len, char **wtf8_ptr, size_t *wtf8_len_ptr) {
        return uv_utf16_to_wtf8(utf16, utf16_len, wtf8_ptr, wtf8_len_ptr);
    }

    inline ssize_t wtf8_length_as_utf16(const char *wtf8) {
        return uv_wtf8_length_as_utf16(wtf8);
    }

    inline void wtf8_to_utf16(const char *utf8, uint16_t *utf16, size_t utf16_len) {
        return uv_wtf8_to_utf16(utf8, utf16, utf16_len);
    }

#endif
}

#endif //LIBUVCXX_UTILITIES_H
