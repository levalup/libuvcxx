//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include <iostream>

#include "uvcxx/inner//base.h"
#include "uvcxx/utils/assert.h"

static int count = 0;
static int copy = 0;

struct V {
    int v = 0;

    V(const V &that) {
        ++count;
        ++copy;
        this->v = that.v;
    }

    V &operator=(const V &that) {
        ++copy;
        this->v = that.v;
        return *this;
    }

    V() {
        ++count;
    }

    ~V() {
        --count;
    }
};

class Shared : public uvcxx::shared_raw_base_t<V> {
public:
    using self = Shared;
    using supper = uvcxx::shared_raw_base_t<V>;

    using supper::supper;

    Shared() : supper(std::make_shared<V>()) {}
};

class Pointer : public uvcxx::pointer_raw_base_t<V> {
public:
    using self = Shared;
    using supper = uvcxx::pointer_raw_base_t<V>;

    using supper::supper;
};

class Inherit : public uvcxx::inherit_raw_base_t<V> {
public:
    using self = Shared;
    using supper = uvcxx::inherit_raw_base_t<V>;

    using supper::supper;
};

class Extend : public uvcxx::extend_raw_base_t<V> {
public:
    using self = Shared;
    using supper = uvcxx::extend_raw_base_t<V>;

    using supper::supper;

    auto &v() { return raw()->v; }

    [[nodiscard]]
    const auto &v() const { return raw()->v; }
};

int main() {
    {
        Shared a = nullptr;
        Shared b;
        uvcxx_assert(!a && b);
        a = std::move(b);
        uvcxx_assert(a && !b);
    }

    std::cout << "count = " << count << std::endl;

    {
        Pointer a = nullptr;
        Pointer b;
        uvcxx_assert(!a && b);
        a = std::move(b);
        uvcxx_assert(a && !b);
    }

    std::cout << "count = " << count << std::endl;

    {
        Inherit a;
        Inherit b;
        b.v = 1;
        uvcxx_assert(!a.v && b.v);
        a = std::move(b);
        uvcxx_assert(a.v && !b.v);
    }

    std::cout << "count = " << count << std::endl;

    {
        Extend a;
        Extend b;
        b.v() = 1;
        uvcxx_assert(!a.v() && b.v());
        a = std::move(b);
        uvcxx_assert(a.v() && !b.v());
    }

    std::cout << "count = " << count << std::endl;
    std::cout << "copy = " << copy << std::endl;

    uvcxx_assert(count == 0);

    return 0;
}
