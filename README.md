# libuvcxx

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

## 2. Schedule

### Loop [OK]

### Req [OK]
### Handle [OK]

### Timer [OK]
### Prepare [OK]
### Check [OK]
### Idle [OK]
### Poll [OK]
### TCP [OK]
### Signal [ing]

### Async [OK]

### File system operations [OK: 90%]

> Note: `readdir`, `scandir` require additional encapsulation to achieve C++style.
>     So the interfaces are not currently supported and will be introduced in the future.

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
