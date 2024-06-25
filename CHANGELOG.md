# libuvcxx changelog

--------------------------------

## v0.0.4

> Since: 2024-06-25

### New features

- Cover about `[99%]` APIs of `libuv`.
  - Add `uv::os` wrapper for `uv_os_xxx`.
  - Add most of miscellaneous utilities.

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

- The inheritance relationship of objects is basically stable, but there are still some interfaces that need to be encapsulated, and the details of encapsulation may also change.

### Bug fix

### Break changes

### Notice

### Future plan

- Encapsulate some necessary classes and add usage examples.
