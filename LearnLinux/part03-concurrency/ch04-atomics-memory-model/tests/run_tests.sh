#!/usr/bin/env bash
# 第三部分第 4 章（原子操作与内存模型入门）验收测试 —— 先于示例实现写定（测试先行）。
# 断言面：
#   ex01 atomic 计数与 mutex 计数同为精确 1000000（同一程序内对照计时，只断言正确性）；
#   ex02 atomic_flag 自旋锁保护普通变量，计数精确；
#   ex03 发布-订阅：acquire 读到标志后，普通数据必须完整（值逐字、总和校验）；
#   ex04 无锁栈：并发 push/pop 后件数与总和守恒（1000 件 / 500500）。
set -u
cd "$(dirname "$0")/.."
BIN="examples/build"
pass=0; fail=0
ok() { echo "PASS: $1"; pass=$((pass+1)); }
ng() { echo "FAIL: $1"; fail=$((fail+1)); }
has() { echo "$1" | grep -qF "$2"; }

if cmake -S examples -B "$BIN" >/dev/null 2>&1 && cmake --build "$BIN" >/dev/null 2>&1; then
  ok "build"
else ng "build"; echo "passed=$pass failed=$fail"; exit 1; fi

o=$(timeout 60 "$BIN/ex01_atomic_counter")
if has "$o" "atomic counter = 1000000 (expected 1000000)" && has "$o" "mutex counter = 1000000 (expected 1000000)"; then
  ok "ex01 atomic == mutex == 1000000"
else ng "ex01: $o"; fi

o=$(timeout 60 "$BIN/ex02_spinlock")
if has "$o" "spinlock counter = 1000000 (expected 1000000)"; then
  ok "ex02 spinlock counter exact"
else ng "ex02: $o"; fi

o=$(timeout 15 "$BIN/ex03_publish_subscribe")
if has "$o" "received: 111 222 333 444" && has "$o" "verified sum = 1110"; then
  ok "ex03 publish-subscribe delivers data intact"
else ng "ex03: $o"; fi

o=$(timeout 30 "$BIN/ex04_lockfree_stack")
if has "$o" "popped = 1000 (expected 1000)" && has "$o" "popped sum = 500500 (expected 500500)"; then
  ok "ex04 lock-free stack conserves count and sum"
else ng "ex04: $o"; fi

echo "----"; echo "passed=$pass failed=$fail"
[ $fail -eq 0 ]
