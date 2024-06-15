//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_PLATFORM_H
#define LIBUVCXX_PLATFORM_H

/**
 * Platform
 * including
 *  UVCXX_OS_WINDOWS
 *  UVCXX_OS_MAC
 *  UVCXX_OS_LINUX
 *  UVCXX_OS_IOS
 *  UVCXX_OS_ANDROID
 */
#if defined(__ANDROID__)
#   define UVCXX_OS_ANDROID 1
#   define UVCXX_OS_WINDOWS 0
#   define UVCXX_OS_MAC     0
#   define UVCXX_OS_LINUX   1
#   define UVCXX_OS_IOS     0
#elif defined(__WINDOWS__) || defined(_WIN32) || defined(WIN32) || defined(_WIN64) || \
    defined(WIN64) || defined(__WIN32__) || defined(__TOS_WIN__)
#   define UVCXX_OS_ANDROID 0
#   define UVCXX_OS_WINDOWS 1
#   define UVCXX_OS_MAC     0
#   define UVCXX_OS_LINUX   0
#   define UVCXX_OS_IOS     0
#elif defined(__MACOSX) || defined(__MACOS_CLASSIC__) || defined(__APPLE__) || defined(__apple__)

#include "TargetConditionals.h"

#if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
#   define UVCXX_OS_ANDROID 0
#   define UVCXX_OS_WINDOWS 0
#   define UVCXX_OS_MAC     0
#   define UVCXX_OS_LINUX   0
#   define UVCXX_OS_IOS     1
#elif TARGET_OS_MAC
#   define UVCXX_OS_ANDROID 0
#   define UVCXX_OS_WINDOWS 0
#   define UVCXX_OS_MAC     1
#   define UVCXX_OS_LINUX   0
#   define UVCXX_OS_IOS     0
#else
//#   error "Unknown Apple platform"
#   define UVCXX_OS_ANDROID 0
#   define UVCXX_OS_WINDOWS 0
#   define UVCXX_OS_MAC     0
#   define UVCXX_OS_LINUX   0
#   define UVCXX_OS_IOS     0
#endif
#elif defined(__linux__) || defined(linux) || defined(__linux) || defined(__LINUX__) || \
    defined(LINUX) || defined(_LINUX)
#   define UVCXX_OS_ANDROID 0
#   define UVCXX_OS_WINDOWS 0
#   define UVCXX_OS_MAC     0
#   define UVCXX_OS_LINUX   1
#   define UVCXX_OS_IOS     0
#else
//#   error Unknown OS
#   define UVCXX_OS_ANDROID 0
#   define UVCXX_OS_WINDOWS 0
#   define UVCXX_OS_MAC     0
#   define UVCXX_OS_LINUX   0
#   define UVCXX_OS_IOS     0
#endif

#endif //LIBUVCXX_PLATFORM_H
