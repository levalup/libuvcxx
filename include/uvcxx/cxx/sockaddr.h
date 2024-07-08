//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_CXX_SOCKADDR_H
#define LIBUVCXX_CXX_SOCKADDR_H

#include <cstring>
#include <ostream>
#include <regex>

#include <uv.h>

#include "./string.h"
#include "./wrapper.h"
#include "../utils/pencil_box.h"
#include "../utils/standard.h"

/**
 * This header file exposes two macros, `AF_INET` and `AF_INET6`,
 *     which are related to platform implementation. Generally,
 *     these two macros will be available after including `uv.h`.
 */

namespace uvcxx {
    enum class Family : int {
        IPv4 = AF_INET,
        IPv6 = AF_INET6,
    };

    class address_t {
    public:
        enum {
            ANY,
        };

        UVCXX_NODISCARD
        Family family() const {
            return Family(addr()->sa_family);
        }

        UVCXX_NODISCARD
        virtual sockaddr *addr() const = 0;

        UVCXX_NODISCARD
        virtual int len() const = 0;

        UVCXX_NODISCARD
        virtual std::string ip() const = 0;

        UVCXX_NODISCARD
        virtual int port() const = 0;

        UVCXX_NODISCARD
        virtual std::string str() const = 0;

        operator sockaddr *() const { return this->addr(); }
    };

    inline std::ostream &operator<<(std::ostream &out, const address_t &addr) {
        return out << addr.str();
    }

    /**
     * Almost all platforms that support sockets provide an implementation of `htons`,
     *     which is just used here to handle possible boundary cases.
     */
    inline uint16_t builtin_htons(int p) {
        union {
            uint8_t a[2];
            uint16_t b = 0x1234;
        } test;
        if (test.a[0] == 0x12) {
            return p;
        } else {
            return (uint16_t)((((uint16_t)(p) & 0xff00U) >> 8) |
                              (((uint16_t)(p) & 0x00ffU) << 8));
        }
    }

    inline uint16_t builtin_ntohs(int p) {
        return builtin_htons(p);
    }

#pragma push_macro("htons")
#pragma push_macro("ntohs")

#ifndef htons
#define htons builtin_htons
#endif
#ifndef ntohs
#define ntohs builtin_htons
#endif

    class any_address_t : public address_t {
    public:
        any_address_t() {
            std::memset(&m_storage, 0, sizeof(m_storage));
            m_size = sizeof(m_storage);
        }

        any_address_t(const sockaddr *addr, int len) {
            std::memcpy(&m_storage, addr, len);
            m_size = len;
        }

        explicit any_address_t(const sockaddr *addr) {
            if (!addr) return;
            if (addr->sa_family == AF_INET) {   // ipv4
                m_size = sizeof(sockaddr_in);
                std::memcpy(&m_storage, addr, m_size);
            } else if (addr->sa_family == AF_INET6) { // ipv6
                m_size = sizeof(sockaddr_in6);
                std::memcpy(&m_storage, addr, m_size);
            }
        }

        UVCXX_NODISCARD
        sockaddr *addr() const final {
            return (sockaddr *) (&m_storage);
        }

        UVCXX_NODISCARD
        int len() const final {
            return m_size;
        }

        UVCXX_NODISCARD
        std::string ip() const final {
            if (m_storage.ss_family == AF_INET) {   // ipv4
                char dst[UV_IF_NAMESIZE] = {0};
                (void) uv_ip4_name((sockaddr_in *) (&m_storage), dst, sizeof(dst));
                return dst;
            } else if (m_storage.ss_family == AF_INET6) { // ipv6
                char dst[UV_IF_NAMESIZE] = {0};
                (void) uv_ip6_name((sockaddr_in6 *) (&m_storage), dst, sizeof(dst));
                return dst;
            } else {
                return "<unknown>";
            }
        }

        UVCXX_NODISCARD
        int port() const final {
            if (m_storage.ss_family == AF_INET) {   // ipv4
                auto ns_port = ((sockaddr_in *) (&m_storage))->sin_port;
                return ntohs(ns_port);
            } else if (m_storage.ss_family == AF_INET6) { // ipv6
                auto ns_port = ((sockaddr_in6 *) (&m_storage))->sin6_port;
                return ntohs(ns_port);
            } else {
                return 0;
            }
        }

        UVCXX_NODISCARD
        std::string str() const final {
            if (m_storage.ss_family == AF_INET) {   // ipv4
                char ip[UV_IF_NAMESIZE] = {0};
                (void) uv_ip4_name((sockaddr_in *) (&m_storage), ip, sizeof(ip));
                auto ns_port = ((sockaddr_in *) (&m_storage))->sin_port;
                auto h_port = ntohs(ns_port);
                return catstr(ip, ":", h_port);
            } else if (m_storage.ss_family == AF_INET6) { // ipv6
                char ip[UV_IF_NAMESIZE] = {0};
                (void) uv_ip6_name((sockaddr_in6 *) (&m_storage), ip, sizeof(ip));
                auto ns_port = ((sockaddr_in6 *) (&m_storage))->sin6_port;
                auto h_port = ntohs(ns_port);
                return catstr("[", ip, "]:", h_port);
            } else {
                return "<unknown>";
            }
        }

        explicit operator sockaddr_in *() const { return (sockaddr_in *) (&m_storage); }

        explicit operator sockaddr_in6 *() const { return (sockaddr_in6 *) (&m_storage); }

        sockaddr *raddr() { return (sockaddr *) &m_storage; }

        int &rlen() { return m_size; }

    private:
        sockaddr_storage m_storage{};
        int m_size = 0;
    };

    class IPv4 : public address_t {
    public:
        IPv4(string ip, int port) {
            UVCXX_APPLY_STRICT(uv_ip4_addr(ip, port, &m_addr), "failed to set address ", ip, " ", port);
        }

        IPv4(decltype(address_t::ANY), int port) {
            UVCXX_APPLY_STRICT(uv_ip4_addr("0.0.0.0", port, &m_addr), "failed to set address 0.0.0.0 ", port);
        }

        UVCXX_NODISCARD
        sockaddr *addr() const final {
            return (struct sockaddr *) (&m_addr);
        }

        UVCXX_NODISCARD
        int len() const final {
            return int(sizeof(m_addr));
        }

        UVCXX_NODISCARD
        std::string ip() const final {
            char dst[UV_IF_NAMESIZE] = {0};
            (void) uv_ip4_name(*this, dst, sizeof(dst));
            return dst;
        }

        UVCXX_NODISCARD
        int port() const final {
            auto ns_port = m_addr.sin_port;
            return ntohs(ns_port);
        }

        UVCXX_NODISCARD
        std::string str() const final {
            char ip[UV_IF_NAMESIZE] = {0};
            (void) uv_ip4_name(&m_addr, ip, sizeof(ip));
            auto ns_port = m_addr.sin_port;
            auto h_port = ntohs(ns_port);
            return catstr(ip, ":", h_port);
        }

        operator sockaddr_in *() const { return (sockaddr_in *) &m_addr; }

        operator any_address_t() const {
            return {addr(), len()};
        }

    private:
        sockaddr_in m_addr{};
    };

    class IPv6 : public address_t {
    public:
        IPv6(string ip, int port) {
            UVCXX_APPLY_STRICT(uv_ip6_addr(ip, port, &m_addr), "failed to set address ", ip, " ", port);
        }

        IPv6(decltype(address_t::ANY), int port) {
            UVCXX_APPLY_STRICT(uv_ip6_addr("::", port, &m_addr), "failed to set address :: ", port);
        }

        UVCXX_NODISCARD
        sockaddr *addr() const final {
            return (struct sockaddr *) (&m_addr);
        }

        UVCXX_NODISCARD
        int len() const final {
            return int(sizeof(m_addr));
        }

        UVCXX_NODISCARD
        std::string ip() const final {
            char dst[UV_IF_NAMESIZE] = {0};
            (void) uv_ip6_name(*this, dst, sizeof(dst));
            return dst;
        }

        UVCXX_NODISCARD
        int port() const final {
            auto ns_port = m_addr.sin6_port;
            return ntohs(ns_port);
        }

        UVCXX_NODISCARD
        std::string str() const final {
            char ip[UV_IF_NAMESIZE] = {0};
            (void) uv_ip6_name(&m_addr, ip, sizeof(ip));
            auto ns_port = m_addr.sin6_port;
            auto h_port = ntohs(ns_port);
            return catstr("[", ip, "]:", h_port);
        }

        operator sockaddr_in6 *() const { return (sockaddr_in6 *) &m_addr; }

        operator any_address_t() const {
            return {addr(), len()};
        }

    private:
        sockaddr_in6 m_addr{};
    };

#pragma pop_macro("htons")
#pragma pop_macro("ntohs")

    inline any_address_t make_address(string ip, int port) {
        static const std::regex ipv6(
                R"(^\s*((([0-9A-Fa-f]{1,4}:){7}([0-9A-Fa-f]{1,4}|:))|(([0-9A-Fa-f]{1,4}:){6}(:[0-9A-Fa-f]{1,4}|)"
                R"(((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3})|:))|(([0-9A-Fa-f)"
                R"(]{1,4}:){5}(((:[0-9A-Fa-f]{1,4}){1,2})|:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4])"
                R"(\d|1\d\d|[1-9]?\d)){3})|:))|(([0-9A-Fa-f]{1,4}:){4}(((:[0-9A-Fa-f]{1,4}){1,3})|((:[0-9A-Fa-f)"
                R"(]{1,4})?:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(()"
                R"([0-9A-Fa-f]{1,4}:){3}(((:[0-9A-Fa-f]{1,4}){1,4})|((:[0-9A-Fa-f]{1,4}){0,2}:((25[0-5]|2[0-4]\)"
                R"(d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){2}(((:[)"
                R"(0-9A-Fa-f]{1,4}){1,5})|((:[0-9A-Fa-f]{1,4}){0,3}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-)"
                R"(5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){1}(((:[0-9A-Fa-f]{1,4}){1,6})|((:)"
                R"([0-9A-Fa-f]{1,4}){0,4}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)"
                R"()){3}))|:))|(:(((:[0-9A-Fa-f]{1,4}){1,7})|((:[0-9A-Fa-f]{1,4}){0,5}:((25[0-5]|2[0-4]\d|1\d\d)"
                R"(|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:)))(%.+)?\s*$)");
        static const std::regex ipv4(
                R"(^(25[0-5]|2[0-4]\d|[0-1]?\d?\d)(\.(25[0-5]|2[0-4]\d|[0-1]?\d?\d)){3}$)");
        if (std::regex_match(ip.c_str, ipv4)) {
            return IPv4(ip, port);
        }
        if (std::regex_match(ip.c_str, ipv6)) {
            return IPv6(ip, port);
        }
        throw uvcxx::errcode(UV_EPERM, "can not tell it's IPv4 or IPv6 address: \"", ip, "\"");
    }
}

#endif //LIBUVCXX_CXX_SOCKADDR_H
