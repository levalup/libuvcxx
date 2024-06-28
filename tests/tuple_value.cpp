//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include <iostream>

#include "uvcxx/utils/tuple.h"
#include "uvcxx/utils/assert.h"

class Moveable {
public:
    int v;

    Moveable() = default;

    Moveable(const Moveable &) = delete;

    Moveable &operator=(const Moveable &) = delete;

    Moveable(Moveable &&) UVCXX_NOEXCEPT = default;

    Moveable &operator=(Moveable &&) UVCXX_NOEXCEPT = default;
};

class NotMoveable {
public:
    int v;

    NotMoveable() = default;

    NotMoveable(NotMoveable &&) UVCXX_NOEXCEPT = delete;

    NotMoveable &operator=(NotMoveable &&) UVCXX_NOEXCEPT = delete;
};

#define V_RETURN(T) std::declval<T(*)()>()()

int &f(int &a) { return a; }

int main() {
    using namespace uvcxx;
    {
        using T = int;
        T v;
        T &rv = v;
        const T &crv = v;
        T *pv = &v;
        const T *cpv = &v;

        uvcxx_assert((std::is_same<decltype(tuple_value<T>(v)), T>::value));
        uvcxx_assert((std::is_same<decltype(tuple_value<T &>(rv)), decltype(std::ref(v))>::value));
        uvcxx_assert((std::is_same<decltype(tuple_value<const T &>(crv)), decltype(std::cref(v))>::value));
        uvcxx_assert((std::is_same<decltype(tuple_value<T *>(pv)), T *>::value));
        uvcxx_assert((std::is_same<decltype(tuple_value<const T *>(cpv)), const T *>::value));

        uvcxx_assert((std::is_same<decltype(tuple_value<T>(V_RETURN(T))), T>::value));
        uvcxx_assert((std::is_same<decltype(tuple_value<T>(V_RETURN(T & ))), T>::value));
        uvcxx_assert((std::is_same<decltype(tuple_value<T>(V_RETURN(const T&))), T>::value));

        uvcxx_assert((std::is_same<decltype(tuple_value<T *>(V_RETURN(T * ))), T *>::value));
        uvcxx_assert((std::is_same<decltype(tuple_value<const T *>(V_RETURN(T * ))), const T *>::value));
        uvcxx_assert((std::is_same<decltype(tuple_value<const T *>(V_RETURN(const T*))), const T*>::value));

        uvcxx_assert((std::is_same<decltype(tuple_value<T &>(V_RETURN(T & ))),
                decltype(std::ref(v))>::value));
        uvcxx_assert((std::is_same<decltype(tuple_value<const T &>(V_RETURN(T & ))),
                decltype(std::cref(v))>::value));
        uvcxx_assert((std::is_same<decltype(tuple_value<const T &>(V_RETURN(const T&))),
                             decltype(std::cref(v))>::value));
    }

    {
        using T = Moveable;
        T v;
        T &rv = v;
        const T &crv = v;
        T *pv = &v;
        const T *cpv = &v;

        uvcxx_assert((std::is_same<decltype(tuple_value<T>(v)), T &&>::value));
        uvcxx_assert((std::is_same<decltype(tuple_value<T &>(rv)), decltype(std::ref(v))>::value));
        uvcxx_assert((std::is_same<decltype(tuple_value<const T &>(crv)), decltype(std::cref(v))>::value));
        uvcxx_assert((std::is_same<decltype(tuple_value<T *>(pv)), T *>::value));
        uvcxx_assert((std::is_same<decltype(tuple_value<const T *>(cpv)), const T *>::value));

        uvcxx_assert((std::is_same<decltype(tuple_value<T>(V_RETURN(T))), T &&>::value));
        uvcxx_assert((std::is_same<decltype(tuple_value<T>(V_RETURN(T & ))), T &&>::value));
        // uvcxx_assert((std::is_same<decltype(tuple_value<T>(V_RETURN(const T&))), T>::value)); //< illegal

        uvcxx_assert((std::is_same<decltype(tuple_value<T *>(V_RETURN(T * ))), T *>::value));
        uvcxx_assert((std::is_same<decltype(tuple_value<const T *>(V_RETURN(T * ))), const T *>::value));
        uvcxx_assert((std::is_same<decltype(tuple_value<const T *>(V_RETURN(const T*))), const T*>::value));

        uvcxx_assert((std::is_same<decltype(tuple_value<T &>(V_RETURN(T & ))),
                decltype(std::ref(v))>::value));
        uvcxx_assert((std::is_same<decltype(tuple_value<const T &>(V_RETURN(T & ))),
                decltype(std::cref(v))>::value));
        uvcxx_assert((std::is_same<decltype(tuple_value<const T &>(V_RETURN(const T&))),
                             decltype(std::cref(v))>::value));
    }

    {
        using T = NotMoveable;
        T v;
        T &rv = v;
        const T &crv = v;
        T *pv = &v;
        const T *cpv = &v;

        uvcxx_assert((std::is_same<decltype(tuple_value<T>(v)), T &>::value));
        uvcxx_assert((std::is_same<decltype(tuple_value<T &>(rv)), decltype(std::ref(v))>::value));
        uvcxx_assert((std::is_same<decltype(tuple_value<const T &>(crv)), decltype(std::cref(v))>::value));
        uvcxx_assert((std::is_same<decltype(tuple_value<T *>(pv)), T *>::value));
        uvcxx_assert((std::is_same<decltype(tuple_value<const T *>(cpv)), const T *>::value));

        uvcxx_assert((std::is_same<decltype(tuple_value<T>(V_RETURN(T))), const T &>::value));
        uvcxx_assert((std::is_same<decltype(tuple_value<T>(V_RETURN(T & ))), T &>::value));
        uvcxx_assert((std::is_same<decltype(tuple_value<T>(V_RETURN(const T&))), const T&>::value));

        uvcxx_assert((std::is_same<decltype(tuple_value<T *>(V_RETURN(T * ))), T *>::value));
        uvcxx_assert((std::is_same<decltype(tuple_value<const T *>(V_RETURN(T * ))), const T *>::value));
        uvcxx_assert((std::is_same<decltype(tuple_value<const T *>(V_RETURN(const T*))), const T*>::value));

        uvcxx_assert((std::is_same<decltype(tuple_value<T &>(V_RETURN(T & ))),
                decltype(std::ref(v))>::value));
        uvcxx_assert((std::is_same<decltype(tuple_value<const T &>(V_RETURN(T & ))),
                decltype(std::cref(v))>::value));
        uvcxx_assert((std::is_same<decltype(tuple_value<const T &>(V_RETURN(const T&))),
                             decltype(std::cref(v))>::value));
    }
}
