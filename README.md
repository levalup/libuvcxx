# libuvcxx

[![License](https://img.shields.io/badge/License-MIT-blue.svg)](
    LICENSE)
[![libuv](https://img.shields.io/badge/libuv-v1.48.0-green?logo=libuv&logoColor=green)](
    https://github.com/libuv/libuv)
[![C++](https://img.shields.io/badge/C++-11-%23512BD4.svg?logo=C%2B%2B&logoColor=%23512BD4)](
    https://en.cppreference.com/w/cpp/17)
[![CMake](https://img.shields.io/badge/CMake-v3.9-%23064F8C?logo=cmake&logoColor=%23064F8C)](
    https://cmake.org)

[![Build](https://github.com/levalup/libuvcxx/actions/workflows/build.yml/badge.svg)](
    https://github.com/levalup/libuvcxx/actions/workflows/build.yml)
[![Release](https://github.com/levalup/libuvcxx/actions/workflows/release.yml/badge.svg)](
    https://github.com/levalup/libuvcxx/actions/workflows/release.yml)

> C++ wrapper for libuv, header only.

## 1. Usage

Copy `include` into your project.  
Of course, you also need to configure `libuv` properly.  
Enjoy using it.  

`libuvcxx` supports single header file usage.
You can find `uvcxx-single.h` in the [release packages](https://github.com/levalup/libuvcxx/releases).

```cpp
#include <uvcxx.h>

int main() {
    uv::timer_t().start(1000, 1000).call([](){
        std::cout << "Hello~~~" << std::endl;
        throw uvcxx::close_handle();
    });
    return uv::default_loop().run();
}
```

The `data` field for context has already been occupied.
Please note that this field should not be used anymore.

No explicit `close` operation is required, except when the `handle` is `running`.
The `handle` can continue to work without any external references, so that explicit `close' is needed at this time.

See [lifecycle.md](docs/lifecycle.md) for more details.

## 2. Compatibility

`libuvcxx` requires at least `C++11` and is also compatible with the new features in `C++14` and `C++17`.

> Tested and passed in gcc `4.8.5` with `libuv` `v1.44.2` on `CentOS7`.

`libuvcxx` can be compatible with libuv: `>= 1.0.0, <= 1.48.0`.
Provided by [test_libuv.sh](scripts/test_libuv.sh).

> Notice: As the development proceeds, compatibility may change with new features.

`libuvcxx` covers `[100%]` all `306` APIs described in the [libuv doc](https://docs.libuv.org/en/v1.x/).
Provided by [libuv_api_coverage.py](scripts/libuv_api_coverage.py).

See [coverage.md](docs/coverage.md) for more details.

## 3. Schedule

More examples and documentation are pending to be added.  
As well as higher-level encapsulations for some common usage scenarios.

## 4. Exception

`libuvcxx` uses exception to handle exceptions.

Most interfaces that don't often encounter errors will throw an `uvcxx::errcode` exception upon failure.
This type of exception is thrown to facilitate the flexible use of interfaces in `promise` or `callback` contexts.

Certain interfaces that frequently encounter errors will not throw exceptions.
In such cases, the success of the interface call should be judged by the return value, for example, `uv::spawn`.

If you're not sure whether an interface will throw an exception, please directly refer to the implementation code of the corresponding interface.
After all, we are an open-source, header-only library.

You can use the macro definition `UVCXX_NO_EXCEPTION` before `#include <uvcxx.h>` to prevent interfaces from throwing exceptions.
However, in the current version, exceptions are still used to handle some transactions.
Future versions may consider finding suitable ways to completely eliminate the use of exceptions.

## 5. Adjusted API

Most APIs have been changed from `uv_xxx` to `uv::xxx`, 
and are placed in the necessary C++ classes according to their types.

Such as
- `uv_handle_t` -> `uv::handle_t`
- `uv_fs_open` -> `uv::fs::open`
- `uv_default_loop` -> `uv::default_loop`
- `uv_close` -> `uv::handle_t::close`
.

However, some APIs still cannot be fully migrated to C++ with consistent naming,
so the following lists the interface names with obvious differences (this may not cover all cases).

- `uv_fs_get_type` -> `uv::fs_t::fs_type`
- `uv_read_start` -> `uv::stream_t::alloc` + `uv::stream_t::read_start`
- `uv_os_environ` -> `uv::os::get_environ`
