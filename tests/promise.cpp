//
// Created by Levalup.
// L.eval: Let programmer get rid of only work jobs.
//

#if defined(_MSC_VER)
#pragma warning(disable: 4702)
#endif

#include "uvcxx/utils/assert.h"
#include "uvcxx/utils/promise.h"


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

    promise<> p0;
    promise<int> p1;
    promise<int, int> p2;

    promise_emitter<> pe0_1(p0);
    promise_emitter<> pe0_2;

    promise_emitter<int> pe1_1(p1);
    promise_emitter<int> pe1_2;

    promise_emitter<int, int> pe2_1(p2);
    promise_emitter<int, int> pe2_2;
    {
        promise_cast<promise<>, float> pc0_1_1(p0, wrapper_0_1_1);
        promise_cast<promise<>, float> pc0_1_2(wrapper_0_1_1);
    }
    {
        promise_cast<promise<>, float> pc0_1_1(p0, wrapper_0_1_2);
        promise_cast<promise<>, float> pc0_1_2(wrapper_0_1_2);
    }
    {
        promise_cast<promise<int>> pc1_0_1(p1, wrapper_1_0_1);
        promise_cast<promise<int>> pc1_0_2(wrapper_1_0_1);
    }
    {
        promise_cast<promise<int>> pc1_0_1(p1, wrapper_1_0_2);
        promise_cast<promise<int>> pc1_0_2(wrapper_1_0_2);
    }
    {
        promise_cast<promise<int>, float, float> pc1_2_1(p1, wrapper_1_2_1);
        promise_cast<promise<int>, float, float> pc1_2_2(wrapper_1_2_1);
    }
    {
        promise_cast<promise<int>, float, float> pc1_2_1(p1, wrapper_1_2_2);
        promise_cast<promise<int>, float, float> pc1_2_2(wrapper_1_2_2);
    }
    {
        promise_cast<promise<int, int>, float, float> pc2_2_1(p2, wrapper_2_2);
        promise_cast<promise<int, int>, float, float> pc2_2_2(wrapper_2_2);
    }

    promise<Moveable> pz;

    promise_emitter<Moveable> pm_z_1(pz);
    promise_emitter<Moveable> pm_z_2;
    {
        promise_cast<promise<int>, Moveable> pm_1_z_1(p1, wrapper_1_m_1);
        promise_cast<promise<int>, Moveable> pm_1_z_2(wrapper_1_m_1);
    }
    {
        promise_cast<promise<int>, Moveable> pm_1_z_1(p1, wrapper_1_m_2);
        promise_cast<promise<int>, Moveable> pm_1_z_2(wrapper_1_m_2);
    }
    {
        promise_cast<promise<Moveable>, int> pm_z_1_1(pz, wrapper_m_1_1);
        promise_cast<promise<Moveable>, int> pm_z_1_2(wrapper_m_1_1);
    }
    {
        promise_cast<promise<Moveable>, int> pm_z_1_1(pz, wrapper_m_1_2);
        promise_cast<promise<Moveable>, int> pm_z_1_2(wrapper_m_1_2);
    }
    {
        promise_cast<promise<Moveable>, Moveable> pm_z_z_1(pz, wrapper_m_m_1);
        promise_cast<promise<Moveable>, Moveable> pm_z_z_2(wrapper_m_m_1);
    }
    {
        promise_cast<promise<Moveable>, Moveable> pm_z_z_1(pz, wrapper_m_m_2);
        promise_cast<promise<Moveable>, Moveable> pm_z_z_2(wrapper_m_m_2);
    }
    {
        promise<int> pn = nullptr;
        promise_emitter<int> pm_n = nullptr;
        promise_cast<promise<int>, int> pc_n = nullptr;
    }
    {
        promise<int&> p;
        promise_emitter<int&> pe;
        promise_cast<promise<int&>, int &> pc([](int &r) -> int & { return r; });
    }
}

void test_call() {
    using namespace uvcxx;
    // bind handler and emit
    {
        {
            promise<> p;
            p.then(nullptr);
            p.then([]() {});

            promise_emitter<> pe1(p);
            promise_emitter<> pe2;
            pe1.resolve();
            pe2.resolve();

            promise_cast<decltype(p), int> pc1(p, [](int) {});
            promise_cast<decltype(p), int> pc2([](int) {});
            pc1.resolve(0);
            pc2.resolve(0);
        }
        {
            promise<int> p;
            p.then(nullptr);
            p.then([]() {});
            p.then([](int) {});
            p.then([](const int&) {});

            promise_emitter<int> pe1(p);
            promise_emitter<int> pe2;
            pe1.resolve(0);
            pe2.resolve(0);

            promise_cast<decltype(p), int> pc1(p, [](int) { return 0; });
            promise_cast<decltype(p), int> pc2([](int) { return 0; });
            pc1.resolve(0);
            pc2.resolve(0);
        }
        {
            promise<Moveable> p;
            p.then(nullptr);
            p.then([]() {});
            p.then([](Moveable) {});
            p.then([](const Moveable &) {});

            promise_emitter<Moveable> pe1(p);
            promise_emitter<Moveable> pe2;
            pe1.resolve(0);
            pe2.resolve(0);

            promise_cast<decltype(p), int> pc1(p, [](int) -> Moveable { return 0; });
            promise_cast<decltype(p), int> pc2([](int) -> Moveable { return 0; });
            pc1.resolve(0);
            pc2.resolve(0);
        }
    }
    // features
    {
        int count = 0;
        int chain = 0;
        promise<int> p;
        p.then([&]() { ++count; });

        {
            promise_emitter<int> proxy(p);
            proxy.resolve(0);
            uvcxx_assert(count == 1);
        }

        p.then([&]() { ++chain; }); //< add chain event handler

        {
            promise_emitter<int> proxy(p);
            proxy.resolve(0);
            uvcxx_assert(count == 2 && chain == 1);
        }
        {
            promise_cast<promise<int>, int> proxy(p, [](int) -> int { return 0; });
            proxy.resolve(0);
            uvcxx_assert(count == 3 && chain == 2);
        }
        {
            p.then(nullptr);    //< clear handler
            promise_emitter<int> proxy(p);
            proxy.resolve(0);
            uvcxx_assert(count == 3 && chain == 2);
        }
    }
}

void test_except() {
    using namespace uvcxx;
    // bind handler and emit
    {
        promise<int> p;
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

        promise_emitter<int> pe(p);

        pe.reject<std::logic_error>("test");

        try {
            throw std::logic_error("test");
        } catch (...) {
            pe.reject(std::current_exception());
        }

        try {
            throw std::logic_error("test");
        } catch (...) {
            pe.reject();
        }
    }
    // features
    {
        // exception chain
        {
            int chain1 = 0;
            int chain2 = 0;
            std::string msg;

            promise<int> p;
            p.except([&]() {
                ++chain1;
            });

            promise_emitter<int>(p).reject<std::logic_error>("test");

            uvcxx_assert(chain1 == 1);

            p.except([&]() {
                ++chain2;
            });

            p.except([&](const std::exception &) {
                msg = "test";
            });

            promise_emitter<int>(p).reject<std::logic_error>("test");

            uvcxx_assert(chain1 == 2 && chain2 == 1);
            uvcxx_assert(msg == "test");
        }

        // catch exception
        {
            int catch_void = 0;
            int catch_base = 0;
            int catch_logic = 0;
            promise<int> p;

            p.except([&]() {
                ++catch_void;
            });

            p.except<std::logic_error>([&]() {
                ++catch_logic;
            });

            p.except<std::exception>([&]() {
                ++catch_base;
            });

            promise_emitter<int>(p).reject<std::logic_error>("test");
            uvcxx_assert(catch_void == 1 && catch_logic == 1 && catch_base == 0);

            promise_emitter<int>(p).reject<std::runtime_error>("test");
            uvcxx_assert(catch_void == 2 && catch_logic == 1 && catch_base == 1);
        }

        // rethrow exception catch, throw exception in handler
        {
            constexpr auto max_rethrow = 1;

            int catch_void = 0;
            int catch_logic = 0;
            promise<int> p;

            p.except<std::logic_error>([&]() {
                ++catch_logic;
                throw std::logic_error("rethrow");
            });

            p.except([&]() {
                ++catch_void;
                throw std::logic_error("rethrow");
            });

            promise_emitter<int>(p).reject<std::runtime_error>("test");
            uvcxx_assert(catch_logic == max_rethrow && catch_void == 1);
        }

        // clear handler chain
        {
            int count = 0;
            promise<int> p;

            p.except([&]() {
                ++count;
            });

            promise_emitter<int>(p).reject<std::logic_error>("test");
            uvcxx_assert(count == 1);

            p.except(nullptr);  //< clear handler chain

            promise_emitter<int>(p).reject<std::logic_error>("test");
            uvcxx_assert(count == 1);
        }
    }
}

void test_finally() {
    using namespace uvcxx;
    // bind handler and emit
    {
        promise<int> p;
        p.finally(nullptr);
        p.finally([]() {});

        promise_emitter<int> pe(p);
        pe.finalize();
    }
    // features
    {
        // limit finalize
        {
            int count = 0;
            promise<int> p;
            p.finally([&]() { ++count; });

            promise_emitter<int> pe(p);

            pe.finalize();
            uvcxx_assert(count == 1);

            pe.finalize();
            uvcxx_assert(count == 1);
        }
        // handler chain
        {
            int count = 0;
            int chain = 0;
            promise<int> p;
            p.finally([&]() { ++count; });
            p.finally([&]() { ++chain; });

            promise_emitter<int> pe(p);

            pe.finalize();
            uvcxx_assert(count == 1 && chain == 1);
        }
        // clear handler chain
        {
            int count = 0;
            promise<int> p;
            p.finally([&]() { ++count; });

            promise_emitter<int> pe(p);

            p.finally(nullptr);
            pe.finalize();
            uvcxx_assert(count == 0);
        }
    }
}

void test_future() {
    using namespace uvcxx;

    // type support
    {
        {
            promise<int> p;
            auto q = p.get_future();
            promise_emitter<int>(p).resolve(1);
            uvcxx_assert(std::get<0>(q.get()) == 1);
        }
        {
            promise<Moveable> p;
            auto q = p.get_future();
            promise_emitter<Moveable>(p).resolve(Moveable(1));
            uvcxx_assert(std::get<0>(q.get()).v == 1);
        }
    }
    // usage
    {
        promise<int> p;

        auto future = p.get_future();

        auto backend = std::async(std::launch::async, [&]() {
            promise_emitter<int>(p).resolve(1);
        });

        uvcxx_assert(std::get<0>(future.get()) == 1);

        backend.wait();
    }

}

int main() {
    test_constructible();
    test_call();
    test_except();
    test_finally();

    test_future();

    return 0;
}
