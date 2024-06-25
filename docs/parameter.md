# libuv API parameter wrapper

This section mainly describes the usage of commonly used parameter in libuv to make them more user-friendly.  
For example:

- the `const char *` parameter is replaced by the `uvcxx::string`;
- the `uv_buf_t` buffer parameter is replaced by `uvcxx::buffer`.

The new types are compatible with the original types and also support more convenient types in C++ and others.

## uvcxx::string

> Including uvcxx::string_view

This type is used to receive common input string types and convert them into C-style strings.

Functions like

```cpp
void open(const char *path);
```

will be modified to

```cpp
void open(uvcxx::string path);
```

, which can support various ways of invocation as follows:

```cpp
std::string cpp_str = "cpp_str";
const char *c_str = "c_str";

open(cpp_str);
open(c_str);
open("literal string");
open(nullptr);
```

`uvcxx::string_view` supports non-`\0`-terminated strings and accepts more input types.

Following is the specific supported parameter types.

|       Type       | `string` | `string_view` |
|:----------------:|:--------:|:-------------:|
|  std::nullptr_t  |    ✅️    |      ✅️       |
|   std::string    |    ✅️    |      ✅️       |
|   const char *   |    ✅️    |      ✅️       |
| `literal string` |    ✅️    |      ✅️       |
| std::string_view |    ❌️    |      ✅️       |

## uvcxx::buffer

> Including uvcxx::mutable_buffer

This type is used to receive common types used for buffers and convert them into `uv_buf_t`.

`libuvcxx` uses `uvcxx::mutable_buffer` to represent a read buffer, and `uvcxx::buffer` to represent a write buffer.

Functions like

```cpp
void read(const uv_buf_t bufs[], unsigned int nbufs);
void write(const uv_buf_t bufs[], unsigned int nbufs);
```

will be modified to

```cpp
void read(uvcxx::mutable_buffer buf);
void read(std::initializer_list<uvcxx::mutable_buffer> bufs);
void write(uvcxx::buffer buf);
void write(std::initializer_list<uvcxx::buffer> bufs);
```

, which can support various ways of invocation as follows:

```cpp
uv::buf_t uvcxx_buf(24);
uv_buf_t uv_buf = uv_buf_init(new char[24], 24);
std::vector<std::byte> std_vec(24);
std::array<std::byte, 24> std_arr;
std::string std_str = "std string";

read(uvcxx_buf);
read(uv_buf);
read(std_vec);
read(std_arr);
read({uvcxx_buf, uv_buf});

write(uvcxx_buf);
write(uv_buf);
write(std_vec);
write(std_str);
write("literal string");
write({uvcxx_buf, uv_buf});

int int_val;
write(uvcxx::buffer_to(int_val));
```

Following is the specific supported parameter types.

|           Type           | `buffer` | `mutable_buffer` |
|:------------------------:|:--------:|:----------------:|
|      std::nullptr_t      |    ✅️    |        ✅️        |
|     const uv_buf_t *     |    ✅️    |        ✅️        |
|         uv_buf_t         |    ✅️    |        ✅️        |
|       uv::buf_t &        |    ✅️    |        ✅️        |
|      std::vector &       |    ✅️    |        ✅️        |
|    { void *, size_t }    |    ✅️    |        ✅️        |
| { const void *, size_t } |    ✅️    |        ❌️        |
|       std::array &       |    ❌️    |        ✅️        |
|        T (&)\[\]         |    ❌️    |        ✅️        |
|      std::string &       |    ✅️    |        ❌️        |
|     `literal string`     |    ✅️    |        ❌️        |
|    std::string_view &    |    ✅️    |        ❌️        |
|     `mutable_buffer`     |    ✅️    |        ✅️        |

Finally, there are also some helper functions that convert more types for buffer usage.

```cpp
template<typename T> inline mutable_buffer buffer_to(T &buf);
template<typename T> inline mutable_buffer buffer_to(T *buf);
template<typename T> inline buffer buffer_to(const T &buf);
template<typename T> inline buffer buffer_to(const T *buf);
```

Notice: `T` of `buffer_to` must be is a [trivial type](https://en.cppreference.com/w/cpp/named_req/TrivialType).
