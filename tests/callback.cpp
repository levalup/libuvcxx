//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#if defined(_MSC_VER)
#pragma warning(disable: 4702)
#endif

#include <future>

#include "uvcxx/utils/assert.h"
#include "uvcxx/utils/callback.h"


class Copyable {
public:
    int v;

    Copyable() = default;
};

class Moveable {
public:
    int v;

    Moveable() = default;

    UVCXX_EXPLICIT_FALSE Moveable(int v) : v(v) {}

    Moveable(const Moveable &) = delete;

    Moveable &operator=(const Moveable &) = delete;

    Moveable(Moveable &&) UVCXX_NOEXCEPT = default;

    Moveable &operator=(Moveable &&) UVCXX_NOEXCEPT = default;
};

void test_constructible() {
    using namespace uvcxx;

    auto wrapper_0_1_1 = [](float) -> std::tuple<> {
        return {};
    };

    auto wrapper_0_1_2 = [](float) -> void {
    };

    auto wrapper_1_0_1 = []() -> std::tuple<int> {
        return {};
    };

    auto wrapper_1_0_2 = []() -> int {
        return 0;
    };

    auto wrapper_1_2_1 = [](float a, float b) -> std::tuple<int> {
        return std::make_tuple(int(a + b));
    };

    auto wrapper_1_2_2 = [](float a, float b) -> int {
        return int(a + b);
    };

    auto wrapper_2_2 = [](float a, float b) -> std::tuple<int, int> {
        return std::make_tuple(int(a + b), int(a - b));
    };

    auto wrapper_m_1_1 = [](int v) -> std::tuple<Moveable> {
        return std::make_tuple(Moveable(v));
    };

    auto wrapper_m_1_2 = [](int v) -> Moveable {
        return {v};
    };

    auto wrapper_1_m_1 = [](const Moveable &m) -> std::tuple<int> {
        return std::make_tuple(m.v);
    };

    auto wrapper_1_m_2 = [](const Moveable &m) -> int {
        return m.v;
    };

    auto wrapper_m_m_1 = [](const Moveable &m) -> std::tuple<Moveable> {
        return std::make_tuple(Moveable(m.v));
    };

    auto wrapper_m_m_2 = [](const Moveable &m) -> Moveable {
        return {m.v};
    };

    callback<> p0;
    callback<int> p1;
    callback<int, int> p2;

    callback_emitter<> pe0_1(p0);
    callback_emitter<> pe0_2;

    callback_emitter<int> pe1_1(p1);
    callback_emitter<int> pe1_2;

    callback_emitter<int, int> pe2_1(p2);
    callback_emitter<int, int> pe2_2;
    {
        callback_cast<callback<>, float> pc0_1_1(p0, wrapper_0_1_1);
        callback_cast<callback<>, float> pc0_1_2(wrapper_0_1_1);
    }
    {
        callback_cast<callback<>, float> pc0_1_1(p0, wrapper_0_1_2);
        callback_cast<callback<>, float> pc0_1_2(wrapper_0_1_2);
    }
    {
        callback_cast<callback<int>> pc1_0_1(p1, wrapper_1_0_1);
        callback_cast<callback<int>> pc1_0_2(wrapper_1_0_1);
    }
    {
        callback_cast<callback<int>> pc1_0_1(p1, wrapper_1_0_2);
        callback_cast<callback<int>> pc1_0_2(wrapper_1_0_2);
    }
    {
        callback_cast<callback<int>, float, float> pc1_2_1(p1, wrapper_1_2_1);
        callback_cast<callback<int>, float, float> pc1_2_2(wrapper_1_2_1);
    }
    {
        callback_cast<callback<int>, float, float> pc1_2_1(p1, wrapper_1_2_2);
        callback_cast<callback<int>, float, float> pc1_2_2(wrapper_1_2_2);
    }
    {
        callback_cast<callback<int, int>, float, float> pc2_2_1(p2, wrapper_2_2);
        callback_cast<callback<int, int>, float, float> pc2_2_2(wrapper_2_2);
    }

    callback<Moveable> pz;

    callback_emitter<Moveable> pm_z_1(pz);
    callback_emitter<Moveable> pm_z_2;
    {
        callback_cast<callback<int>, Moveable> pm_1_z_1(p1, wrapper_1_m_1);
        callback_cast<callback<int>, Moveable> pm_1_z_2(wrapper_1_m_1);
    }
    {
        callback_cast<callback<int>, Moveable> pm_1_z_1(p1, wrapper_1_m_2);
        callback_cast<callback<int>, Moveable> pm_1_z_2(wrapper_1_m_2);
    }
    {
        callback_cast<callback<Moveable>, int> pm_z_1_1(pz, wrapper_m_1_1);
        callback_cast<callback<Moveable>, int> pm_z_1_2(wrapper_m_1_1);
    }
    {
        callback_cast<callback<Moveable>, int> pm_z_1_1(pz, wrapper_m_1_2);
        callback_cast<callback<Moveable>, int> pm_z_1_2(wrapper_m_1_2);
    }
    {
        callback_cast<callback<Moveable>, Moveable> pm_z_z_1(pz, wrapper_m_m_1);
        callback_cast<callback<Moveable>, Moveable> pm_z_z_2(wrapper_m_m_1);
    }
    {
        callback_cast<callback<Moveable>, Moveable> pm_z_z_1(pz, wrapper_m_m_2);
        callback_cast<callback<Moveable>, Moveable> pm_z_z_2(wrapper_m_m_2);
    }
    {
        callback<int> pn = nullptr;
        callback_emitter<int> pm_n = nullptr;
        callback_cast<callback<int>, int> pc_n = nullptr;
    }
    {
        callback<int&> p;
        callback_emitter<int&> pe;
        callback_cast<callback<int&>, int &> pc([](int &r) -> int & { return r; });
    }
}

void test_call() {
    using namespace uvcxx;
    // bind handler and emit
    {
        {
            callback<> p;
            p.call(nullptr);
            p.call([]() {});

            callback_emitter<> pe1(p);
            callback_emitter<> pe2;
            pe1.emit();
            pe2.emit();

            callback_cast<decltype(p), int> pc1(p, [](int) {});
            callback_cast<decltype(p), int> pc2([](int) {});
            pc1.emit(0);
            pc2.emit(0);
        }
        {
            callback<int> p;
            p.call(nullptr);
            p.call([]() {});
            p.call([](int) {});
            p.call([](const int&) {});

            callback_emitter<int> pe1(p);
            callback_emitter<int> pe2;
            pe1.emit(0);
            pe2.emit(0);

            callback_cast<decltype(p), int> pc1(p, [](int) { return 0; });
            callback_cast<decltype(p), int> pc2([](int) { return 0; });
            pc1.emit(0);
            pc2.emit(0);
        }
        {
            callback<Moveable> p;
            p.call(nullptr);
            p.call([]() {});
            p.call([](Moveable) {});
            p.call([](const Moveable&) {});

            callback_emitter<Moveable> pe1(p);
            callback_emitter<Moveable> pe2;
            pe1.emit(0);
            pe2.emit(0);

            callback_cast<decltype(p), int> pc1(p, [](int) -> Moveable { return 0; });
            callback_cast<decltype(p), int> pc2([](int) -> Moveable { return 0; });
            pc1.emit(0);
            pc2.emit(0);
        }
    }
    // features
    {
        int count = 0;
        int chain = 0;
        callback<int> p;
        p.call([&]() { ++count; });

        {
            callback_emitter<int> proxy(p);
            proxy.emit(0);
            uvcxx_assert(count == 1);
        }

        p.call([&]() { ++chain; }); //< add chain event handler

        {
            callback_emitter<int> proxy(p);
            proxy.emit(0);
            uvcxx_assert(count == 2 && chain == 1);
        }
        {
            callback_cast<callback<int>, int> proxy(p, [](int) -> int { return 0; });
            proxy.emit(0);
            uvcxx_assert(count == 3 && chain == 2);
        }
        {
            p.call(nullptr);    //< clear handler
            callback_emitter<int> proxy(p);
            proxy.emit(0);
            uvcxx_assert(count == 3 && chain == 2);
        }
    }
}

void test_except() {
    using namespace uvcxx;
    // bind handler and emit
    {
        callback<int> p;
        p.except(nullptr);

        p.except([](const std::exception_ptr &) {});
        p.except([](const std::exception_ptr &) -> bool { return false; });
        p.except([]() {});
        p.except([]() -> bool { return false; });

        p.except([](const std::exception &) {});
        p.except<std::logic_error>([]() {});
        p.except<std::logic_error>([](const std::logic_error &) {});
        // p.except([](const std::exception &) -> bool { return false; }); //< deprecated
        // p.except<std::logic_error>([]() -> bool { return false; });     //< deprecated
        // p.except<std::logic_error>([](const std::logic_error &) -> bool { return false; }); //< deprecated

        callback_emitter<int> pe(p);

        (void) pe.raise<std::logic_error>("test");

        try {
            throw std::logic_error("test");
        } catch (...) {
            (void) pe.raise(std::current_exception());
        }

        try {
            throw std::logic_error("test");
        } catch (...) {
            (void) pe.raise();
        }

        (void) pe.raise<std::logic_error>("test");
    }
    // features
    {
        // exception chain
        {
            int chain1 = 0;
            int chain2 = 0;
            std::string msg;

            callback<int> p;
            p.except([&]() {
                ++chain1;
            });

            (void) callback_emitter<int>(p).raise<std::logic_error>("test");

            uvcxx_assert(chain1 == 1);

            p.except([&]() {
                ++chain2;
            });

            p.except([&](const std::exception &) {
                msg = "test";
            });

            (void) callback_emitter<int>(p).raise<std::logic_error>("test");

            uvcxx_assert(chain1 == 2 && chain2 == 1);
            uvcxx_assert(msg == "test");
        }

        // catch exception
        {
            int catch_void = 0;
            int catch_base = 0;
            int catch_logic = 0;
            callback<int> p;

            p.except([&]() {
                ++catch_void;
            });

            p.except<std::logic_error>([&]() {
                ++catch_logic;
            });

            p.except<std::exception>([&]() {
                ++catch_base;
            });

            (void) callback_emitter<int>(p).raise<std::logic_error>("test");
            uvcxx_assert(catch_void == 1 && catch_logic == 1 && catch_base == 0);

            (void) callback_emitter<int>(p).raise<std::runtime_error>("test");
            uvcxx_assert(catch_void == 2 && catch_logic == 1 && catch_base == 1);
        }

        // rethrow exception catch, throw exception in handler
        {
            constexpr auto max_rethrow = 1;

            int catch_void = 0;
            int catch_logic = 0;
            callback<int> p;

            p.except<std::logic_error>([&]() {
                ++catch_logic;
                throw std::logic_error("rethrow");
            });

            p.except([&]() {
                ++catch_void;
                throw std::logic_error("rethrow");
            });

            (void) callback_emitter<int>(p).raise<std::runtime_error>("test");
            uvcxx_assert(catch_logic == max_rethrow && catch_void == 1);
        }

        // clear handler chain
        {
            int count = 0;
            callback<int> p;

            p.except([&]() {
                ++count;
            });

            (void) callback_emitter<int>(p).raise<std::logic_error>("test");
            uvcxx_assert(count == 1);

            p.except(nullptr);  //< clear handler chain

            (void) callback_emitter<int>(p).raise<std::logic_error>("test");
            uvcxx_assert(count == 1);
        }

        // watch exception
        {
            int watch_count = 0;
            int except_count = 0;
            callback<int> p;

            p.watch([&](const std::exception_ptr &) -> bool {
                ++watch_count;
                return false;
            });

            p.except([&]() {
                ++except_count;
            });

            (void) callback_emitter<int>(p).raise<std::logic_error>("test");
            uvcxx_assert(watch_count == 1 && except_count == 1);

            p.except(nullptr);  //< clear handler chain, but keep watch

            (void) callback_emitter<int>(p).raise<std::logic_error>("test");
            uvcxx_assert(watch_count == 2 && except_count == 1);
        }
    }
}

void test_finally() {
    using namespace uvcxx;
    // bind handler and emit
    {
        callback<int> p;
        p.finally(nullptr);
        p.finally([]() {});

        callback_emitter<int> pe(p);
        pe.finalize();
    }
    // features
    {
        // limit finalize
        {
            int count = 0;
            callback<int> p;
            p.finally([&]() { ++count; });

            callback_emitter<int> pe(p);

            pe.finalize();
            uvcxx_assert(count == 1);

            pe.finalize();
            uvcxx_assert(count == 1);
        }
        // handler chain
        {
            int count = 0;
            int chain = 0;
            callback<int> p;
            p.finally([&]() { ++count; });
            p.finally([&]() { ++chain; });

            callback_emitter<int> pe(p);

            pe.finalize();
            uvcxx_assert(count == 1 && chain == 1);
        }
        // clear handler chain
        {
            int count = 0;
            callback<int> p;
            p.finally([&]() { ++count; });

            callback_emitter<int> pe(p);

            p.finally(nullptr);
            pe.finalize();
            uvcxx_assert(count == 0);
        }
    }
}

void test_queue() {
    using namespace uvcxx;

    // type support
    {
        {
            callback<int> p;
            auto q = p.get_queue();
            callback_emitter<int>(p).emit(1);
            uvcxx_assert(std::get<0>(q.pop().second) == 1);
        }
        {
            callback<Moveable> p;
            auto q = p.get_queue();
            callback_emitter<Moveable>(p).emit(Moveable(1));
            uvcxx_assert(std::get<0>(q.pop().second).v == 1);
        }
    }
    // usage
    {
        callback<int> p;

        auto queue = p.get_queue();

        auto backend = std::async(std::launch::async, [&]() {
            callback_emitter<int> pe(p);
            pe.emit(1);
            pe.emit(2);
            pe.emit(3);
            pe.emit(4);
            pe.emit(5);
            pe.emit(6);
            pe.finalize();  // finalize to close queue
        });

        uvcxx_assert(queue.pop() == std::make_pair(true, std::make_tuple(1)));
        uvcxx_assert(queue.pop() == std::make_pair(true, std::make_tuple(2)));
        uvcxx_assert(queue.pop() == std::make_pair(true, std::make_tuple(3)));

        int v = 4;
        for (auto &kv : queue) {
            uvcxx_assert(kv == std::make_tuple(v++));
        }

        auto fin = queue.pop();
        uvcxx_assert(!fin.first);

        backend.wait();
    }
}

int main() {
    test_constructible();
    test_call();
    test_except();
    test_finally();

    test_queue();

    return 0;
}
