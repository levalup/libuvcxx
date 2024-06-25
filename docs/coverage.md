# Libuv API coverage

## Auto calculated API coverage

Following is the output of [libuv_api_coverage.py](../scripts/libuv_api_coverage.py).

```text
[INFO] Section "Error handling" ... [OK]
[INFO] Section "Version-checking macros and functions" ... [OK]
[INFO] Section "uv_loop_t - Event loop" ... [OK]
[INFO] Section "uv_handle_t - Base handle" ... [OK]
[INFO] Section "uv_req_t - Base request" ... [OK]
[INFO] Section "uv_timer_t - Timer handle" ... [OK]
[INFO] Section "uv_prepare_t - Prepare handle" ... [OK]
[INFO] Section "uv_check_t - Check handle" ... [OK]
[INFO] Section "uv_idle_t - Idle handle" ... [OK]
[INFO] Section "uv_async_t - Async handle" ... [OK]
[INFO] Section "uv_poll_t - Poll handle" ... [OK]
[INFO] Section "uv_signal_t - Signal handle" ... [OK]
[INFO] Section "uv_process_t - Process handle" ... [OK]
[INFO] Section "uv_stream_t - Stream handle" ... [OK]
[INFO] Section "uv_tcp_t - TCP handle" ... [OK]
[INFO] Section "uv_pipe_t - Pipe handle" ... [OK]
[INFO] Section "uv_tty_t - TTY handle" ... [OK]
[INFO] Section "uv_udp_t - UDP handle" ... [OK]
[INFO] Section "uv_fs_event_t - FS Event handle" ... [OK]
[INFO] Section "uv_fs_poll_t - FS Poll handle" ... [OK]
[INFO] Section "File system operations" ... [OK]
[INFO] Section "Thread pool work scheduling" ... [OK]
[INFO] Section "DNS utility functions" ... [OK]
[INFO] Section "Shared library handling" ... [OK]
[INFO] Section "Threading and synchronization utilities" ... [97%]
[WARN] Miss:   - uv_thread_self
[INFO] Section "Miscellaneous utilities" ... [OK]
[INFO] Section "Metrics operations" ... [OK]
[INFO] Total API coverage [99%]
```

## API version limit

The version limitations of most interfaces are already aligned with the original version requirements of `libuv`.

Therefore, using a lower version of `libuv` would not cause undefined function issues.

However, it is not possible to use functions that were not implemented in the lower version of `libuv`.

Well, `libuvcxx` has implemented some commonly used functions for lower versions to ensure the availability across all
versions.  
Such as:

- Obtaining members of `loop`, `fs`, `handle`, `req`, and `process`, like `uv_xxx_get_data`.
- The synchronous version of `uv::random`.
- `uv::sleep`.
- `UV_VERSION_HEX`.
