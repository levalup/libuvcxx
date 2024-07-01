//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include <iostream>

#include "uvcxx.h"

void init_buffer(void *data, size_t size) {
    int c = 0;
    auto dest = (char *) data;
    for (size_t i = 0; i < size; ++i) {
        c = (c + 1) % 10;
        dest[i] = char('0' + c);
    }
}

std::string mb(uvcxx::mutable_buffer buf) {
    return {(char *) buf.buf.base, buf.buf.len};
}

std::string b(uvcxx::buffer buf) {
    return {(char *) buf.buf.base, buf.buf.len};
}

int main() {
    uv::buf_t uvcxx_buf(3);
    char uv_buf_data[4] = {};
    auto uv_buf = uv_buf_init(uv_buf_data, 4);
    std::vector<int16_t> std_vec(3);
    int16_t c_arr[4] = {};
    std::array<int32_t, 2> std_arr{};
    std::string std_str = "123456789";
    const char c_str1[] = "1234567";
    char c_str2[7] = {};
    int64_t ii = {};

    init_buffer(uvcxx_buf.data(), uvcxx_buf.size());
    init_buffer(uv_buf.base, uv_buf.len);
    init_buffer(std_vec.data(), std_vec.size() * sizeof(int16_t));
    init_buffer(c_arr, sizeof(c_arr));
    init_buffer(std_arr.data(), std_arr.size() * sizeof(int32_t));
    init_buffer(c_str2, sizeof(c_str2));
    c_str2[6] = '\0';
    init_buffer(&ii, sizeof(ii));

    uvcxx_assert(b(nullptr).empty());
    uvcxx_assert(mb(nullptr).empty());

    uvcxx_assert(b(uvcxx_buf) == "123");
    uvcxx_assert(b(&uv_buf) == "1234");
    uvcxx_assert(b(uv_buf) == "1234");
    uvcxx_assert(b({c_arr, 5}) == "12345");
    uvcxx_assert(b(std_vec) == "123456");
    uvcxx_assert(b(std_str) == "123456789");
    uvcxx_assert(b(c_str1) == "1234567");
    uvcxx_assert(b(c_str2) == "123456");    // different to `mutable_buffer`
    uvcxx_assert(b("vwxyz") == "vwxyz");
    uvcxx_assert(b("vwxyz\0") == "vwxyz");
    uvcxx_assert(b(uvcxx::buffer_to(ii)) == "12345678");

    uvcxx_assert(mb(uvcxx_buf) == "123");
    uvcxx_assert(mb(&uv_buf) == "1234");
    uvcxx_assert(mb(uv_buf) == "1234");
    uvcxx_assert(mb({c_arr, 5}) == "12345");
    uvcxx_assert(mb(std_vec) == "123456");
    uvcxx_assert(mb(c_arr) == "12345678");
    uvcxx_assert(mb(std_arr) == "12345678");
    uvcxx_assert(mb(c_str2) == std::string("123456\0", 7));    // different to `buffer`
    uvcxx_assert(mb(uvcxx::buffer_to(ii)) == "12345678");

    auto constructible_buffer_c_arr = std::is_constructible<uvcxx::buffer, decltype(c_arr)>::value;
    uvcxx_assert(!constructible_buffer_c_arr);

    auto constructible_buffer_std_arr = std::is_constructible<uvcxx::buffer, decltype(std_arr)>::value;
    uvcxx_assert(!constructible_buffer_std_arr);

    auto constructible_mutable_buffer_std_str =
            std::is_constructible<uvcxx::mutable_buffer, decltype(std_str)>::value;
    uvcxx_assert(!constructible_mutable_buffer_std_str);

    auto constructible_mutable_buffer_c_str1 =
            std::is_constructible<uvcxx::mutable_buffer, decltype(c_str1)>::value;
    uvcxx_assert(!constructible_mutable_buffer_c_str1);

    auto constructible_mutable_buffer_literal_str =
            std::is_constructible<uvcxx::mutable_buffer, decltype("vwxyz")>::value;
    uvcxx_assert(!constructible_mutable_buffer_literal_str);

    return 0;
}
