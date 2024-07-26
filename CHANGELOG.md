# libuvcxx changelog

--------------------------------

## v0.1.4

> Date: 2024-07-26

### New features

- Using the new lifecycle management method, it is now necessary to explicitly call detach to switch lifecycles.
  - This avoids the additional mental overhead caused by implicit lifecycle transitions.

## Bug fix

## Break changes

- Change error exception name.
  - `E_EAGAIN` -> `E_AGAIN`
  - `E_EADDRINUSE` -> `E_ADDRINUSE`
  - `E_EBADF` -> `E_BADF`
  - `E_ENOTSOCK` -> `E_NOTSOCK`

--------------------------------

## v0.1.3

> Date: 2024-07-02

### New features

- Add `callback` method on handles.
    - Use `callback` method to directly obtain callback for such as `async`, `check` or `timer`.
    - Use `listen_callback` method to obtain the callback for stream's `listen`.
    - You can set up handling functions independently before performing any work.
    - This can be very useful when operations may involve multiple starts and stops.
- Add examples.
- `pipe`, `tcp` change state to `detached` after `connected`.
    - This is an experimental feature and may be adjusted before the official release.
- Add `uvcxx/cxx/sockaddr.h` to load/dump `sockaddr_*`.

## Bug fix

- Fix the variable usage error in the `lib_t::open(string)` method.
- Fix `pipe_connect` forgot set data field.
- Fix some cases where header files are not included.

## Break changes

- Change return value of `signal_t::start_oneshot`.
    - `callback<int>` -> `promise<int>`
- Remove `acceptable_stream` abstract class layer.
    - Practical experience proves that this abstraction layer does not significantly solve the problem of usability,
      but also introduces some unnecessary complexity.
- Remove `stream_t::accept`.
    - Overload the function in specific subtypes to avoid ambiguous semantics inherent in it.
- Remove meaningless `accpt` implementations in some stream subclasses.

--------------------------------

## v0.1.2

> Date: 2024-07-01

### New features

- Add CI workflows.
    - Test latest ubuntu/windows gcc, clang and cl.
    - Test classic GCC 4.8.5 with libuv 1.44.2.
    - Test GCC 5, 6, 7, 8, 9, 10, 11, 12, latest.
    - Test GCC with -std=11, 14, 17.
    - Test MacOS with Xcode(Clang).
    - Test MinGW.
    - Test MSVC with /std:11, 14, 17.
    - Test single header.

### Bug fix

- Fix the issue that the generated single header file may vary in different environments,
    - which leads to compilation errors.

--------------------------------

## v0.1.1

> Date: 2024-06-28

### New features

- Tested and passed on gcc `4.8.5` with `libuv` `v1.44.2`.
- Enhanced the capabilities of `promise/callback`.
    - It can properly handle movable but non-copyable objects.
    - It supports passing references.
- Add `merge.py` to merge the code of uvcxx into a single header file.
    - You can find `uvcxx-single.h` in the release package.

### Bug fix

- Fix std::async UB with no policy in early C++ standard.
    - Add `std::launch::async`.

### Break changes

- The release of associated resources, such as `uv_fs_req_cleanup`, will be called before the `finally` of
  the `promise`.
    - This adjustment is necessary for compilation on gcc `4.8.x`.
    - Except for the deprecated usage, this modification has no impact in most scenarios.
    - Bote: do not save related resources in the `then` function and use in `finally`.

- Add `status` check on `handle` callback and change callback type.
    - `fs_event` start return `callback<const char *, uv_fs_event>`.
    - `fs_poll` start return `callback<const uv_stat_t *, const uv_stat_t *>`.
    - `poll` start return `callback<int>`.
    - `stream` listen return `callback<>`.
    - Handle status issue with `except<uvcxx::errcode>(...)`.

--------------------------------

## v0.1.0

> Date: 2024-06-27

### New features

- Cover about all APIs of `libuv`.
    - Add `uv::os` wrapper for `uv_os_xxx`.
    - Add most of miscellaneous utilities.
- Downwards to C++11, while keep compatibility to C++14 and C++17.
    - The minimum GCC version tested is 5.4.
    - CMAKE_CXX_STANDARD in (11 14 17), have passed the tests on MSVC (19.34), GCC (11), and Clang (15).
    - More compatibility tests are pending.

### Bug fix

- Fixed known issues that caused double free of resources when moving wrapper objects.

### Break changes

- Rename `uvcxx::buffer_like` to `uvcxx::buffer`.
- Rename `uvcxx::mutable_buffer_like` to `uvcxx::mutable_buffer`.

--------------------------------

## v0.0.3

> Date: 2024-06-24

### New features

- Add `uvcxx::string`, `uvcxx::string_view` and `uvcxx::buffer_like` to simplify the type conversion.

### Bug fix

- Fix `req` not set data field problem.

### Break changes

- Removed some overloaded versions of `fs::read` and `fs::write`. Simplified by using `buffer_like` type.

--------------------------------

## v0.0.2

> Date: 2024-06-23

### New features

- Cover about `[97%]` APIs of `libuv`.
    - Add `thread`, `cond`, `mutex`, `rwlock`, `sem`, `once` and `barrier`.
    - Add `process`.
    - Finish `fs`.
- Add scripts to calculate coverage and compatibility of `libuv`.
- Designed and implemented a more secure resource lifecycle management.
- Verified compilation in latest ubuntu `gcc` and `clang`, windows `msvc` and `mingw`.
- Verified compilation in GCC `9`, `10`, `11`, `12`.
- Verified compilation in libuv `>= 1.0.0, <= 1.48.0`.

### Bug fix

- `uv::timer_t::get_due_in` implementation error.
- fix `uv::signal_t::start` parameter.

### Break changes

### Future plan

- Add more basic usage examples.
- Finish "Miscellaneous utilities" part.

--------------------------------

## v0.0.1

> Date: 2024-06-16

### New features

- The inheritance relationship of objects is basically stable, but there are still some interfaces that need to be
  encapsulated, and the details of encapsulation may also change.

### Bug fix

### Break changes

### Notice

### Future plan

- Encapsulate some necessary classes and add usage examples.
