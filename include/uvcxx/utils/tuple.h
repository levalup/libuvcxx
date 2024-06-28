//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_TUPLE_H
#define LIBUVCXX_TUPLE_H

#include <type_traits>

namespace uvcxx {
    /**
     * tuple_value solve to situation:
     * 1. wrapper parameter to `std::make_tuple` in: [](T ...v){ return std::make_tuple(tuple_value<T>(v)...); }
     * 2. wrapper return value of function in: std::make_tuple(tuple_value<T>(wrapper(...)));
     * @tparam T
     * @param t
     * @return
     */
    template<typename T, typename std::enable_if<
            std::is_scalar<T>::value,
            int>::type = 0>
    inline auto tuple_value(T t) -> T { return t; }

    template<typename T, typename std::enable_if<
            std::is_rvalue_reference<T>::value,
            int>::type = 0>
    inline auto tuple_value(T t) -> T { return std::move(t); }

    template<typename T, typename std::enable_if<
            std::is_lvalue_reference<T>::value && std::is_const<T>::value,
            int>::type = 0>
    inline auto tuple_value(T t) -> decltype(std::cref(std::declval<T>())) { return std::cref(t); }

    template<typename T, typename std::enable_if<
            std::is_lvalue_reference<T>::value && !std::is_const<T>::value,
            int>::type = 0>
    inline auto tuple_value(T t) -> decltype(std::ref(std::declval<T>())) { return std::ref(t); }

    template<typename T, typename std::enable_if<
            !std::is_reference<T>::value && !std::is_scalar<T>::value && !std::is_move_constructible<T>::value,
            int>::type = 0>
    inline auto tuple_value(T &t) -> T & { return t; }

    template<typename T, typename std::enable_if<
            !std::is_reference<T>::value && !std::is_scalar<T>::value && !std::is_move_constructible<T>::value,
            int>::type = 0>
    inline auto tuple_value(const T &t) -> const T & { return t; }

    template<typename T, typename std::enable_if<
            !std::is_reference<T>::value && !std::is_scalar<T>::value && std::is_move_constructible<T>::value,
            int>::type = 0>
    inline auto tuple_value(T &t) -> T && { return std::move(t); }

    template<typename T, typename std::enable_if<
            !std::is_reference<T>::value && !std::is_scalar<T>::value && std::is_move_constructible<T>::value,
            int>::type = 0>
    inline auto tuple_value(T &&t) -> T && { return std::forward<T>(t); }
}

#endif //LIBUVCXX_TUPLE_H
