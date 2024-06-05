# libuvcxx

> C++ wrapper for libuv, header only.

## 1. Usage

Copy `include` to you project.

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

TODO:
```
Make `uv::async_t` as template class.
Use `uv::async_t<int, float>` to run `send(int, float)` async.
```

### File system operations [OK: 90%]

> In process...

> Note: `readdir`, `scandir` require additional encapsulation to achieve C++style.
>     So the interfaces are not currently supported and will be introduced in the future.
