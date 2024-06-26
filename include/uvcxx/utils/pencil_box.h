//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_PENCIL_BOX_H
#define LIBUVCXX_PENCIL_BOX_H

#include <sstream>

namespace uvcxx {
    inline void catout(std::ostream &) {}

    template<typename Arg, typename ...Args>
    inline void catout(std::ostream &out, const Arg &arg, const Args &...args) {
        catout(out << arg, args...);
    }

    template<typename ...Args>
    inline std::string catstr(const Args &...args) {
        std::ostringstream oss;
        catout(oss, args...);
        return oss.str();
    }
}

#endif //LIBUVCXX_PENCIL_BOX_H
