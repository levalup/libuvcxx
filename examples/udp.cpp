//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include <iostream>

#include "uvcxx/cxx/sockaddr.h"
#include "uvcxx/cxx/to_string.h"
#include "uvcxx/udp.h"
#include "uvcxx/buf.h"
#include "uvcxx/timer.h"
#include "uvcxx/utilities.h"

uvcxx::any_address_t getsockname(const uv::udp_t &tcp) {
    uvcxx::any_address_t name;
    auto len = name.len();
    (void) tcp.getsockname(name, &len);
    return name;
}

uvcxx::any_address_t getpeername(const uv::udp_t &tcp) {
    uvcxx::any_address_t name;
    auto len = name.len();
    (void) tcp.getpeername(name, &len);
    return name;
}

int main() {
    // setting
    std::string localhost = "127.0.0.1";
    int port = 0;
    auto server_time_ms = 1000;

    uv::loop_t server_loop;
    uv::loop_t client_loop;

    // server
    {
        // make address support IPv4 and IPv6
        auto addr = uvcxx::make_address("0.0.0.0", port);
        // if work IPv4 only, simplify code as following
        // uvcxx::IPv4 addr("0.0.0.0", port); // or uvcxx::IPv4(uvcxx::address_t::ANY, port);

        uv::udp_t server(server_loop);
        server.bind(addr, 0);

        auto server_sock = getsockname(server);
        port = server_sock.port();

        std::cout << "[INFO] server start listen: " << server_sock << std::endl;

        uv::buf_t server_buf;
        server.alloc().call([=](size_t size, uv_buf_t *buf) mutable {
            server_buf.resize(size);
            *buf = server_buf;
        });
        server.recv_start().call(
                [=](ssize_t nread, const uv_buf_t *buf, const sockaddr *addr, uv_udp_flags flags) mutable {
                    std::cout << "server read: " << std::string(buf->base, nread) << " with "
                              << uvcxx::to_string(uv_udp_flags(flags)) << " from " << uvcxx::any_address_t(addr)
                              << std::endl;
                }).except([=](const std::exception &e) {
            std::cout << "[ERROR] recv " << addr << " failed. " << e.what() << std::endl;
            throw uvcxx::close_handle();
        });

        std::cout << "[INFO] stop server after " << server_time_ms << "ms" << std::endl;
        uv::timer_t(server_loop).start(server_time_ms, 1).call([=]() mutable {
            server.close(nullptr);
            throw uvcxx::close_handle();    // close timer
        });
    }

    // client
    for (int i = 0; i < 5; ++i) {
        auto addr = uvcxx::make_address(localhost, port);

        auto msg = uvcxx::catstr("hello~", i);

        uv::udp_t client(client_loop, false);
        client.connect(addr);
        client.send(msg, nullptr).then([=]() {
            std::cout << "client write: " << msg << std::endl;
        }).finally([=]() mutable {
            client.close(nullptr);
        });
    }

    auto server_run = std::async(std::launch::async, [&]() {
        server_loop.run();
        std::cout << "[INFO] server exit" << std::endl;
    });

    auto client_run = std::async(std::launch::async, [&]() {
        client_loop.run();
        std::cout << "[INFO] client exit" << std::endl;
    });

    server_run.wait();
    client_run.wait();

    return 0;
}
