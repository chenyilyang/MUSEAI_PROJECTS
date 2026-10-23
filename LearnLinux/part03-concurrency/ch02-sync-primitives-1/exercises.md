# 第 2 章 练习（第三部分）

共 5 题。先自己动手，再到 `solutions/` 对照（文字题答案在 `solutions/answers.md`，代码题答案是可编译运行的源码）。

## 练习 1（找问题：两处临界区错误）

```cpp
std::mutex m1, m2;
Config g_cfg;

void updateFromCan(const Config& c) {          // 线程 A 调用
    std::lock_guard<std::mutex> lk(m1);
    g_cfg = c;
    renderReportToFile(g_cfg);               // 把报告渲染并写文件（耗时约 50ms）
}

Config readSnapshot() {                        // 线程 B 调用
    std::lock_guard<std::mutex> lk(m2);
    return g_cfg;
}
```

这段代码违反了临界区的哪两条纪律？各指出一处，并说明后果与改法。

## 练习 2（改错：裸 lock/unlock 的漏网路径）

```cpp
std::mutex g_m;
long g_balance = 0;

bool withdraw(long amount) {
    g_m.lock();
    if (amount > g_balance) {
        return false;              // ← 注意这条路径
    }
    g_balance -= amount;
    g_m.unlock();
    return true;
}
```

指出 bug 及其后果（为什么它可能潜伏很久才爆），然后用 RAII 改写，并说明改后每条返回路径的解锁由谁保证。

## 练习 3（写代码：线程安全计数器类）

实现 `ThreadSafeCounter` 类：私有数据 `long value_` 与配套 mutex，对外提供 `inc()`、`add(long)`、`value() const`。写主程序：4 个线程各 inc 250000 次，最终打印必须恰为 1000000，不符以非 0 退出码报警。体会"数据与锁同居、接口即防线"的封装方式（注意 const 方法里上锁需要什么）。

## 练习 4（写代码：分片计数器）

实现 `StripedCounter`：内部 8 个分片，每片一把 mutex + 一个子计数；`inc()` 按当前线程 id 哈希选片，`value()` 按固定顺序锁住全部片求和。同样用 4×250000 验证总和精确。说明它相比单锁计数器快在哪、又付出了什么代价。

## 练习 5（思考题）

(a) 列出三个必须使用 unique_lock 而不能只用 lock_guard 的场景，并各说一句理由。
(b) 粗锁在 ex04 里最快，为什么生产系统仍不能无脑粗锁？结合"临界区里夹着一段 IO"举例说明。
(c) 读写锁在"写很频繁"的场景下可能反而不如普通 mutex，试着解释原因（提示：读写锁本身的管理开销与读者/写者的互相等待）。
