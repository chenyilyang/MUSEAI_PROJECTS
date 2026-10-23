// ex04：C++20 专题（三）—— 协程一瞥：手写最小生成器【-std=c++20 构建】
// 协程是"能暂停再恢复的函数"：fib() 在每次 co_yield 处冻结局部状态（a、b）
// 并把控制权还给调用方，下次 resume 从冻结处继续——不用回调、不用手写状态机。
// C++20 只给了协程的"语言底座"（co_yield/co_await/co_return + promise 协议），
// 具体生成器类型要自己拼（std::generator 是 C++23 才进标准库的成品）。
// 这里拼一个最小可用的 Generator<T>：惰性启动、逐个取值、析构时销毁协程帧。
#include <coroutine>
#include <cstdio>

template <typename T>
struct Generator {
    struct promise_type {
        T current;
        Generator get_return_object() {
            return Generator(std::coroutine_handle<promise_type>::from_promise(*this));
        }
        std::suspend_always initial_suspend() { return {}; }        // 惰性：先不跑
        std::suspend_always final_suspend() noexcept { return {}; }
        std::suspend_always yield_value(T v) {
            current = v;
            return {};
        }
        void return_void() {}
        void unhandled_exception() {}
    };

    explicit Generator(std::coroutine_handle<promise_type> h) : h_(h) {}
    ~Generator() { if (h_) h_.destroy(); }
    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;
    Generator(Generator&& o) noexcept : h_(o.h_) { o.h_ = nullptr; }  // 句柄所有权转移

    // 前进一步；还有产出返回 true，并从 value() 取本轮的值。
    bool next() {
        h_.resume();
        return !h_.done();
    }
    T value() const { return h_.promise().current; }

private:
    std::coroutine_handle<promise_type> h_;
};

static Generator<long> fib() {
    long a = 0, b = 1;
    for (;;) {
        co_yield a;                     // 在此冻结：交出 a，记住 a、b 与执行点
        long t = a + b;
        a = b;
        b = t;
    }
}

int main() {
    Generator<long> g = fib();
    std::printf("fib:");
    bool exact = true;
    const long want[10] = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34};
    for (int i = 0; i < 10; ++i) {
        if (!g.next()) { exact = false; break; }
        std::printf(" %ld", g.value());
        if (g.value() != want[i]) exact = false;
    }
    std::printf("\n");
    return exact ? 0 : 1;
}
