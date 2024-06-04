//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include "uvcxx.h"

int main() {
    uv_getaddrinfo_t req;
    addrinfo hints = {};
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = 0;

    uv::getaddrinfo(
            "baidu.com", nullptr, &hints
    ).then([](addrinfo *ai) {
        char addr[INET6_ADDRSTRLEN];
        for (addrinfo* info = ai; info != nullptr; info = info->ai_next) {
            uv_ip4_name((struct sockaddr_in*)info->ai_addr, addr, sizeof(addr));
            std::cout << "Found address: " << addr << std::endl;
            uv::getnameinfo(info->ai_addr, 0).then([](const char *hostname, const char *service) {
                std::cout << "Name: " << hostname << " " << service << std::endl;
            });
        }
    }).except([](const std::exception &e) {
        std::cout << "can not getaddrinfo: " << e.what() << std::endl;
    });
    uv::default_loop().run();
    return 0;
}