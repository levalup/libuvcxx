//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_API_H
#define LIBUVCXX_API_H

#define UVCXX_EXTERN_C extern "C"

#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__)
#   define UVCXX_DLL_IMPORT __declspec(dllimport)
#   define UVCXX_DLL_EXPORT __declspec(dllexport)
#   define UVCXX_DLL_LOCAL
#else
#   if defined(__GNUC__) && __GNUC__ >= 4
#       define UVCXX_DLL_IMPORT __attribute__((visibility("default")))
#       define UVCXX_DLL_EXPORT __attribute__((visibility("default")))
#       define UVCXX_DLL_LOCAL  __attribute__((visibility("hidden")))
#   else
#       define UVCXX_DLL_IMPORT
#       define UVCXX_DLL_EXPORT
#       define UVCXX_DLL_LOCAL
#   endif
#endif

#if defined(SHARED_UVCXX)
#define UVCXX_API UVCXX_DLL_EXPORT
#else
#define UVCXX_API UVCXX_DLL_IMPORT
#endif

#ifdef __cplusplus
#   define UVCXX_C_API UVCXX_EXTERN_C UVCXX_API
#else
#   define UVCXX_C_API UVCXX_API
#endif

#endif //LIBUVCXX_API_H
