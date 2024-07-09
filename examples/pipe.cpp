//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include <iostream>

#include "uvcxx/pipe.h"
#include "uvcxx/buf.h"
#include "uvcxx/timer.h"
#include "uvcxx/utilities.h"

int main() {
    // setting
    std::string pipe_name = "a.pipe";
    auto server_time_ms = 1000;

    (void) std::remove(pipe_name.c_str());

    uv::loop_t server_loop;
    uv::loop_t client_loop;

    // server
    {
        uv::pipe_t server(server_loop, false);
        server.bind(pipe_name);

        server.listen(128).call([=]() mutable {
            auto conn = server.accept(false);

            uv::buf_t server_buf;
            conn.alloc().call([=](size_t size, uv_buf_t *buf) mutable {
                server_buf.resize(size);
                *buf = server_buf;
            });
            conn.read_start().call([=](ssize_t nread, const uv_buf_t *buf) mutable {
                std::cout << "server read: " << std::string(buf->base, nread) << std::endl;
            }).except<uvcxx::E_AGAIN>([]() {
            }).except<uvcxx::E_EOF>([=]() mutable {
                conn.close(nullptr);
            }).except([=]() mutable {
                conn.close(nullptr);
            });
        }).except([=](const std::exception &e) {
            std::cout << "[ERROR] listen named pipe " << pipe_name << " failed. " << e.what() << std::endl;
            throw uvcxx::close_handle();
        }).finally([=]() {
            (void) std::remove(pipe_name.c_str());
        });

        std::cout << "[INFO] stop server after " << server_time_ms << "ms" << std::endl;
        uv::timer_t(server_loop).start(server_time_ms, 1).call([=]() mutable {
            server.close(nullptr);
            throw uvcxx::close_handle();    // close timer
        });
    }

    // client
    for (int i = 0; i < 5; ++i) {
        auto msg = uvcxx::catstr("hello~", i);

        uv::pipe_t client(client_loop, false);
        client.connect(pipe_name).then([=]() mutable {
            client.write(msg).then([=]() {
                std::cout << "client write: " << msg << std::endl;
            });
            // wait write finish then shutdown
            client.shutdown().finally([=]() mutable {
                client.close(nullptr);
            });
        }).except([=]() mutable {
            std::cout << "[ERROR] can not connect to " << pipe_name << std::endl;
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
