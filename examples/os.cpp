//
// Created by Levalup.
// L.eval = Let programmer get rid of only work jobs.
//

#include <iostream>

#include "uvcxx/os.h"

int main() {
#if UVCXX_SATISFY_VERSION(1, 6, 0)
    std::cout << "homedir = " << uv::os::homedir() << std::endl;
#endif
#if UVCXX_SATISFY_VERSION(1, 9, 0)
    std::cout << "tmpdir = " << uv::os::tmpdir() << std::endl;
    auto passwd = uv::os::get_passwd();
    std::cout << "passwd.username = " << passwd.username << std::endl;
    std::cout << "passwd.uid = " << passwd.uid << std::endl;
    std::cout << "passwd.gid = " << passwd.gid << std::endl;
    std::cout << "passwd.shell = " << passwd.shell << std::endl;
    std::cout << "passwd.homedir = " << passwd.homedir << std::endl;
#endif
#if UVCXX_SATISFY_VERSION(1, 12, 0)
    std::cout << "hostname = " << uv::os::gethostname() << std::endl;
    std::cout << "setenv[ABC] = '123'" << std::endl;
    uv::os::setenv("ABC", "123");
    std::cout << "getenv[ABC] = '" << uv::os::getenv("ABC") << "'" << std::endl;
    std::cout << "unsetenv[ABC]" << std::endl;
    uv::os::unsetenv("ABC");
    std::cout << "getenv[ABC] = '" << uv::os::getenv("ABC") << "'" << std::endl;
#endif
#if UVCXX_SATISFY_VERSION(1, 16, 0)
    std::cout << "ppid = " << uv::os::getppid() << std::endl;
#endif
#if UVCXX_SATISFY_VERSION(1, 18, 0)
    std::cout << "pid = " << uv::os::getpid() << std::endl;
#endif
#if UVCXX_SATISFY_VERSION(1, 23, 0)
    auto pid = uv::os::getpid();
    int priority;
    uv::os::getpriority(pid, &priority);
    std::cout << "get priority = " << priority << std::endl;
    std::cout << "set priority = " << priority << std::endl;
    uv::os::setpriority(pid, priority);
#endif
#if UVCXX_SATISFY_VERSION(1, 25, 0)
    uv_utsname_t uname{};
    uv::os::uname(&uname);
    std::cout << "uname.sysname = " << uname.sysname << std::endl;
    std::cout << "uname.release = " << uname.release << std::endl;
    std::cout << "uname.version = " << uname.version << std::endl;
    std::cout << "uname.machine = " << uname.machine << std::endl;
#endif
#if UVCXX_SATISFY_VERSION(1, 32, 0)
    auto env_items = uv::os::environ();
    for (auto &env: env_items) {
        std::cout << "env[" << env.name << "] = '" << env.value << "'" << std::endl;
    }
#endif
#if UVCXX_SATISFY_VERSION(1, 45, 0)
    auto passwd2 = uv::os::get_passwd(passwd.uid);
    std::cout << "passwd2.username = " << passwd2.username << std::endl;
    std::cout << "passwd2.uid = " << passwd2.uid << std::endl;
    std::cout << "passwd2.gid = " << passwd2.gid << std::endl;
    std::cout << "passwd2.shell = " << passwd2.shell << std::endl;
    std::cout << "passwd2.homedir = " << passwd2.homedir << std::endl;
    auto group = uv::os::get_group(passwd.gid);
    std::cout << "group.groupname = " << group.groupname << std::endl;
    std::cout << "group.gid = " << group.gid << std::endl;
    {
        bool comma = false;
        std::cout << "group.members = ";
        auto member = group.members;
        while (*member) {
            if (comma) std::cout << ",";
            else comma = true;
            std::cout << *member;
            ++member;
        }
        std::cout << std::endl;
    }
#endif
    return 0;
}
