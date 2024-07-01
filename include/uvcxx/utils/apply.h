//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_APPLY_H
#define LIBUVCXX_APPLY_H

#include <cstddef>
#include <tuple>

#include "standard.h"

namespace uvcxx {
#if UVCXX_STD_APPLY

    template<typename F, typename T>
    inline decltype(auto) proxy_apply(F &&f, T &&t) {
        return std::apply(std::forward<F>(f), std::forward<T>(t));
    }

#else
    namespace inner {
        template<typename F, typename Tuple>
        class Apply;

        template<typename FUNC, typename... Args>
        class Apply<FUNC, std::tuple<Args...>> {
        public:
            using Ret = decltype(std::declval<FUNC>()(std::declval<Args>()...));

            template<size_t I, typename F, typename T, typename std::enable_if<I == 0, int>::type = 0>
            Ret apply(F &&f, T &&) {
                return f();
            }

            template<size_t I, typename F, typename T, typename std::enable_if<I == 1, int>::type = 0>
            Ret apply(F &&f, T &&t) {
                return f(std::get<0>(std::forward<T>(t)));
            }

            template<size_t I, typename F, typename T, typename std::enable_if<I == 2, int>::type = 0>
            Ret apply(F &&f, T &&t) {
                return f(std::get<0>(std::forward<T>(t)),
                         std::get<1>(std::forward<T>(t)));
            }

            template<size_t I, typename F, typename T, typename std::enable_if<I == 3, int>::type = 0>
            Ret apply(F &&f, T &&t) {
                return f(std::get<0>(std::forward<T>(t)),
                         std::get<1>(std::forward<T>(t)),
                         std::get<2>(std::forward<T>(t)));
            }

            template<size_t I, typename F, typename T, typename std::enable_if<I == 4, int>::type = 0>
            Ret apply(F &&f, T &&t) {
                return f(std::get<0>(std::forward<T>(t)),
                         std::get<1>(std::forward<T>(t)),
                         std::get<2>(std::forward<T>(t)),
                         std::get<3>(std::forward<T>(t)));
            }

            template<size_t I, typename F, typename T, typename std::enable_if<I == 5, int>::type = 0>
            Ret apply(F &&f, T &&t) {
                return f(std::get<0>(std::forward<T>(t)),
                         std::get<1>(std::forward<T>(t)),
                         std::get<2>(std::forward<T>(t)),
                         std::get<3>(std::forward<T>(t)),
                         std::get<4>(std::forward<T>(t)));
            }

            template<size_t I, typename F, typename T, typename std::enable_if<I == 6, int>::type = 0>
            Ret apply(F &&f, T &&t) {
                return f(std::get<0>(std::forward<T>(t)),
                         std::get<1>(std::forward<T>(t)),
                         std::get<2>(std::forward<T>(t)),
                         std::get<3>(std::forward<T>(t)),
                         std::get<4>(std::forward<T>(t)),
                         std::get<5>(std::forward<T>(t)));
            }

            template<size_t I, typename F, typename T, typename std::enable_if<I == 7, int>::type = 0>
            Ret apply(F &&f, T &&t) {
                return f(std::get<0>(std::forward<T>(t)),
                         std::get<1>(std::forward<T>(t)),
                         std::get<2>(std::forward<T>(t)),
                         std::get<3>(std::forward<T>(t)),
                         std::get<4>(std::forward<T>(t)),
                         std::get<5>(std::forward<T>(t)),
                         std::get<6>(std::forward<T>(t)));
            }

            template<size_t I, typename F, typename T, typename std::enable_if<I == 8, int>::type = 0>
            Ret apply(F &&f, T &&t) {
                return f(std::get<0>(std::forward<T>(t)),
                         std::get<1>(std::forward<T>(t)),
                         std::get<2>(std::forward<T>(t)),
                         std::get<3>(std::forward<T>(t)),
                         std::get<4>(std::forward<T>(t)),
                         std::get<5>(std::forward<T>(t)),
                         std::get<6>(std::forward<T>(t)),
                         std::get<7>(std::forward<T>(t)));
            }

            template<size_t I, typename F, typename T, typename std::enable_if<I == 9, int>::type = 0>
            Ret apply(F &&f, T &&t) {
                return f(std::get<0>(std::forward<T>(t)),
                         std::get<1>(std::forward<T>(t)),
                         std::get<2>(std::forward<T>(t)),
                         std::get<3>(std::forward<T>(t)),
                         std::get<4>(std::forward<T>(t)),
                         std::get<5>(std::forward<T>(t)),
                         std::get<6>(std::forward<T>(t)),
                         std::get<7>(std::forward<T>(t)),
                         std::get<8>(std::forward<T>(t)));
            }

            template<size_t I, typename F, typename T, typename std::enable_if<I == 10, int>::type = 0>
            Ret apply(F &&f, T &&t) {
                return f(std::get<0>(std::forward<T>(t)),
                         std::get<1>(std::forward<T>(t)),
                         std::get<2>(std::forward<T>(t)),
                         std::get<3>(std::forward<T>(t)),
                         std::get<4>(std::forward<T>(t)),
                         std::get<5>(std::forward<T>(t)),
                         std::get<6>(std::forward<T>(t)),
                         std::get<7>(std::forward<T>(t)),
                         std::get<8>(std::forward<T>(t)),
                         std::get<9>(std::forward<T>(t)));
            }

            template<typename F, typename T>
            Ret operator()(F &&f, T &&t) {
                return this->apply<sizeof...(Args)>(std::forward<F>(f), std::forward<T>(t));
            }
        };
    }

    template<typename F, typename T>
    inline auto proxy_apply(F &&f, T &&t) ->
    typename inner::Apply<typename std::decay<F>::type, typename std::decay<T>::type>::Ret {
        using Apply = inner::Apply<typename std::decay<F>::type, typename std::decay<T>::type>;
        return Apply()(std::forward<F>(f), std::forward<T>(t));
    }

#endif
}

#endif //LIBUVCXX_APPLY_H
