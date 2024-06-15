# libuvcxx

[![License](https://img.shields.io/badge/License-MIT-blue.svg?logo=)](LICENSE)
[![libuv](https://img.shields.io/badge/libuv-v1.48.0-green?logo=libuv&logoColor=green)](https://github.com/libuv/libuv)
[![libuv](https://img.shields.io/badge/C++-17-%23512BD4.svg?logo=C%2B%2B&logoColor=%23512BD4)](https://en.cppreference.com/w/cpp/17)
[![libuv](https://img.shields.io/badge/CMake-v3.9-%23064F8C?logo=cmake&logoColor=%23064F8C)](https://cmake.org)

> C++ wrapper for libuv, header only.

## 1. Usage

Copy `include` into your project.  
Of course, you also need to configure `libuv` properly.  
Enjoy using it.  

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

Note that all the `handle` and `request` `data` members are used to store data related to C++ operations, so please do not use them.

If you need to use context, use the capture list of `lambda`.

The `handle`‘s `data` will be recycled after `close`, so be sure to call `close` after the `handle` is confirmed to not be used anymore.

The `request`'s `data` will be recycled after the callback is completed, so most of the time there is no need to consider resource recycling issues.

## 2. Schedule

### Process [delay]

> This section mainly focuses on encapsulating options to facilitate the manipulation of starting processes.

### Thread [ing]

> Convert most operations to C++ interface style.

### File system operations [OK: 90%]

> Note: `readdir`, `scandir` require additional encapsulation to achieve C++style.
>     So the interfaces are not currently supported and will be introduced in the future.

### Others

Most of the interfaces have been encapsulated or the reasons for not encapsulating have been explained.
The next work of `libuvcxx` is mainly about the above content.

## Exception

`libuvcxx` uses exception to handle exceptions.

Most interfaces that don't often encounter errors will throw an `uvcxx::errcode` exception upon failure.
This type of exception is thrown to facilitate the flexible use of interfaces in `promise` or `callback` contexts.

Certain interfaces that frequently encounter errors will not throw exceptions.
In such cases, the success of the interface call should be judged by the return value, for example, uv::process_t::spawn.

If you're not sure whether an interface will throw an exception, please directly refer to the implementation code of the corresponding interface.
After all, we are an open-source, header-only library.

You can use the macro definition `UVCXX_NO_EXCEPTION` before `#include <uvcxx.h>` to prevent interfaces from throwing exceptions.
However, in the current version, exceptions are still used to handle some transactions.
Future versions may consider finding suitable ways to completely eliminate the use of exceptions.

## Adjusted API

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

- `uv_fs_get_type` -> `uv::fs_t::get_fs_type`
- `uv_read_start` -> `uv::stream_t::alloc` + `uv::stream_t::read_start`
