//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#ifndef LIBUVCXX_TEST_STRUCT_H
#define LIBUVCXX_TEST_STRUCT_H

#include <iostream>

#include "uvcxx.h"

struct V {
    static int count;
    int v = 0;
};

void test_v_init(V *v);

void test_v_free(V *v);

class Shared : public uvcxx::shared_raw_base_t<V> {
public:
    using self = Shared;
    using supper = uvcxx::shared_raw_base_t<V>;

    using supper::supper;

    Shared() : supper(make_shared()) {}

private:
    static std::shared_ptr<V> make_shared() {
        auto v = new V;
        test_v_init(v);
        return {v, test_v_free};
    }
};

class Pointer : public uvcxx::pointer_raw_base_t<V> {
public:
    using self = Shared;
    using supper = uvcxx::pointer_raw_base_t<V>;

    using supper::supper;

    Pointer(Pointer &&) = default;

    Pointer &operator=(Pointer &&) = default;

    Pointer() {
        test_v_init(*this);
    }

    ~Pointer() override {
        if (*this) test_v_free(*this);
    }
};

class Inherit : public uvcxx::inherit_raw_base_t<V> {
public:
    using self = Shared;
    using supper = uvcxx::inherit_raw_base_t<V>;

    using supper::supper;

    Inherit(const Inherit &) = delete;

    Inherit &operator=(const Inherit &) = delete;

    Inherit(Inherit &&) = default;

    Inherit &operator=(Inherit &&) = default;

    Inherit() {
        test_v_init(*this);
    }

    ~Inherit() {
        if (v) test_v_free(*this);
    }
};

class Extend : public uvcxx::extend_raw_base_t<V> {
public:
    using self = Shared;
    using supper = uvcxx::extend_raw_base_t<V>;

    using supper::supper;

    Extend(const Extend &) = delete;

    Extend &operator=(const Extend &) = delete;

    Extend(Extend &&) = default;

    Extend &operator=(Extend &&) = default;

    Extend() {
        test_v_init(*this);
    }

    ~Extend() {
        if (raw()->v) test_v_free(*this);
    }
};

inline Shared inline_move_shared() {
    return {};
}

inline Pointer inline_move_pointer() {
    return {};
}

inline Inherit inline_move_inherit() {
    return {};
}

inline Extend inline_move_extend() {
    return {};
}

Shared link_move_shared();

Pointer link_move_pointer();

Inherit link_move_inherit();

Extend link_move_extend();

#endif //LIBUVCXX_TEST_STRUCT_H
