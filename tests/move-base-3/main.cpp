//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include "struct.h"

int main() {
    {
        auto v = inline_move_shared();
        auto c = std::move(v);
        uvcxx_assert(V::count == 1);
        uvcxx_assert(!v);
        uvcxx_assert(((V *) c)->v == 1);
    }
    uvcxx_assert(V::count == 0);
    {
        auto v = inline_move_pointer();
        auto c = std::move(v);
        uvcxx_assert(V::count == 1);
        uvcxx_assert(!v);
        uvcxx_assert(((V *) c)->v == 1);
    }
    uvcxx_assert(V::count == 0);
    {
        auto v = inline_move_inherit();
        auto c = std::move(v);
        uvcxx_assert(V::count == 1);
        uvcxx_assert(((V *) v)->v == 0);
        uvcxx_assert(((V *) c)->v == 1);
    }
    uvcxx_assert(V::count == 0);
    {
        auto v = inline_move_extend();
        auto c = std::move(v);
        uvcxx_assert(V::count == 1);
        uvcxx_assert(((V *) v)->v == 0);
        uvcxx_assert(((V *) c)->v == 1);
    }
    uvcxx_assert(V::count == 0);
    {
        auto v = link_move_shared();
        auto c = std::move(v);
        uvcxx_assert(V::count == 1);
        uvcxx_assert(!v);
        uvcxx_assert(((V *) c)->v == 1);
    }
    uvcxx_assert(V::count == 0);
    {
        auto v = link_move_pointer();
        auto c = std::move(v);
        uvcxx_assert(V::count == 1);
        uvcxx_assert(!v);
        uvcxx_assert(((V *) c)->v == 1);
    }
    uvcxx_assert(V::count == 0);
    {
        auto v = link_move_inherit();
        auto c = std::move(v);
        uvcxx_assert(V::count == 1);
        uvcxx_assert(((V *) v)->v == 0);
        uvcxx_assert(((V *) c)->v == 1);
    }
    uvcxx_assert(V::count == 0);
    {
        auto v = link_move_extend();
        auto c = std::move(v);
        uvcxx_assert(V::count == 1);
        uvcxx_assert(((V *) v)->v == 0);
        uvcxx_assert(((V *) c)->v == 1);
    }
    uvcxx_assert(V::count == 0);
}
