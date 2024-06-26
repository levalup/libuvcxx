//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_STANDARD_H
#define LIBUVCXX_STANDARD_H

// noexcept
#if __cpp_noexcept_function_type >= 201510L || __cplusplus >= 201103L || _MSC_VER >= 1900
#define UVCXX_NOEXCEPT noexcept
#else
#define UVCXX_NOEXCEPT
#endif

// [[noreturn]]
#if __has_cpp_attribute(noreturn) || __cplusplus >= 201103L || _MSC_VER >= 1900
#define UVCXX_NORETURN [[noreturn]]
#else
#define UVCXX_NORETURN
#endif

// [[nodiscard]]
#if __has_cpp_attribute(nodiscard) ||  __cplusplus >= 201703L || _MSC_VER >= 1910
#define UVCXX_NODISCARD [[nodiscard]]
#else
#define UVCXX_NODISCARD
#endif

// Initialized/Generalized lambda captures (init-capture)
#if __cpp_init_captures >= 201304L || __cplusplus >= 201402L || _MSC_VER >= 1900
#define UVCXX_CAPTURE_MOVE(v) v = std::move(v)
#else
#define UVCXX_CAPTURE_MOVE(v) v
#endif

// Initialized/Generalized lambda captures (init-capture)
#if __cpp_init_captures >= 201304L || __cplusplus >= 201402L || _MSC_VER >= 1900
#define UVCXX_CAPTURE_MOVE(v) v = std::move(v)
#else
#define UVCXX_CAPTURE_MOVE(v) v
#endif

// Initialized/Generalized lambda captures (init-capture)
#if __cpp_if_constexpr >= 201606L || __cplusplus >= 201703L || _MSC_VER >= 1911
#define UVCXX_IF_CONSTEXPR constexpr
#else
#define UVCXX_IF_CONSTEXPR
#endif

#endif //LIBUVCXX_STANDARD_H
