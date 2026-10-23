#!/usr/bin/env bash
# 第三部分第 2 章（同步原语一）验收测试 —— 先于示例实现写定（测试先行）。
# 断言面：
#   ex01 第 1 章的共享计数加 mutex 后恰为 1000000；
#   ex02 异常穿过临界区后锁仍被 RAII 释放（后续加锁不挂死）、unique_lock 两段式结果正确；
#   ex03 读写锁下读者永不见中间态（violations = 0），最终 version/value 一致；
#   ex04 三种策略都跑完；两种加锁策略结果精确，无锁版只断言有输出（数值不定）。
# 耗时数值不断言（机器相关），只断言正确性。
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

o=$(timeout 30 "$BIN/ex01_mutex_counter")
if has "$o" "mutex counter = 1000000 (expected 1000000)"; then
  ok "ex01 mutex fixes the race exactly"
else ng "ex01: $o"; fi

o=$(timeout 15 "$BIN/ex02_raii_locks")
if has "$o" "caught: boom" && has "$o" "total after exception path = 42" && has "$o" "final total = 84"; then
  ok "ex02 RAII releases lock on exception; unique_lock two-phase result"
else ng "ex02: $o"; fi

o=$(timeout 30 "$BIN/ex03_rwlock")
if has "$o" "violations = 0" && has "$o" "final: version = 200, value = 2000"; then
  ok "ex03 rwlock: no torn reads, final state consistent"
else ng "ex03: $o"; fi

o=$(timeout 60 "$BIN/ex04_lock_cost")
if echo "$o" | grep -qE "^racy +: shared = [0-9]+ \((wrong|exact -- lucky this time)\), [0-9]+ ms" \
   && has "$o" "coarse lock : shared = 1000000 (exact)" \
   && has "$o" "fine lock   : shared = 1000000 (exact)"; then
  ok "ex04 three strategies complete; locked ones exact"
else ng "ex04: $o"; fi

echo "----"; echo "passed=$pass failed=$fail"
[ $fail -eq 0 ]
