//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_STANDARD_H
#define LIBUVCXX_STANDARD_H

// noexcept
#if __cpp_noexcept_function_type >= 201510L || __cplusplus >= 201103L || _MSC_VER >= 1900
#define UVCXX_STD_NOEXCEPT 1
#define UVCXX_NOEXCEPT noexcept
#else
#define UVCXX_NOEXCEPT
#endif

// [[noreturn]]
#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(noreturn) || __cplusplus >= 201103L || _MSC_VER >= 1900
#define UVCXX_STD_NORETURN 1
#define UVCXX_NORETURN [[noreturn]]
#else
#define UVCXX_NORETURN
#endif
#else
#define UVCXX_NORETURN
#endif

// [[nodiscard]]
#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(nodiscard) || __cplusplus >= 201703L || _MSC_VER >= 1910
#define UVCXX_STD_NODISCARD 1
#define UVCXX_NODISCARD [[nodiscard]]
#else
#define UVCXX_NODISCARD
#endif
#else
#define UVCXX_NODISCARD
#endif

// Initialized/Generalized lambda captures (init-capture)
#if __cpp_init_captures >= 201304L
#define UVCXX_STD_INIT_CAPTURES 1
#define UVCXX_CAPTURE_MOVE(v) v = std::move(v)
#else
#define UVCXX_CAPTURE_MOVE(v) v
#endif

// Initialized/Generalized lambda captures (init-capture)
#if __cpp_if_constexpr >= 201606L
#define UVCXX_STD_IF_CONSTEXPR 1
#define UVCXX_IF_CONSTEXPR constexpr
#else
#define UVCXX_IF_CONSTEXPR
#endif

// std::string_view
#if __cpp_lib_string_view >= 201606L
#define UVCXX_STD_STRING_VIEW 1
#endif

// Class template argument deduction
#if __cpp_deduction_guides >= 201703L
#define UVCXX_STD_DEDUCTION_GUIDES 1
#endif

// std::apply
#if __cpp_lib_apply >= 201603L
#define UVCXX_STD_APPLY 1
#endif

// 	explicit(bool)
#if __cpp_conditional_explicit >= 201806L
#define UVCXX_STD_CONDITIONAL_EXPLICIT 1
#define UVCXX_EXPLICIT_FALSE explicit(false)
#else
#define UVCXX_EXPLICIT_FALSE
#endif

#endif //LIBUVCXX_STANDARD_H
