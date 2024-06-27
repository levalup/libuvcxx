//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#include "struct.h"

int V::count = 0;

void test_v_init(V *v) {
    V::count++;
    uvcxx_assert(v->v == 0);
    v->v++;
}

void test_v_free(V *v) {
    V::count--;
    uvcxx_assert(v->v == 1);
    v->v--;
}

Shared link_move_shared() {
    return {};
}

Pointer link_move_pointer() {
    return {};
}

Inherit link_move_inherit() {
    return {};
}

Extend link_move_extend() {
    return {};
}
