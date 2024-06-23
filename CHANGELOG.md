# libuvcxx changelog

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
