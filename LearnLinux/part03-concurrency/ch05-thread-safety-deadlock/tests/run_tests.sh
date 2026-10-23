#!/usr/bin/env bash
# 第三部分第 5 章（并发安全与死锁）验收测试 —— 先于示例实现写定（测试先行）。
# 断言面：
#   bad_01 普通构建照常跑完（只断言有结果行）；TSan 构建必须报出 data race；
#   bad_02 两线程反序加锁必须真死锁（timeout 124），且双方都打印了"已持有第一把锁"；
#   ex01 std::lock 修复版双向转账完成，总额守恒、两账户回到 1000；
#   ex02 mutex 计数在 TSan 下零报告、退出码 0、结果精确。
# TSan 构建由本脚本以 -fsanitize=thread 直接编译（演示它只是一个编译开关）。
# 运行加 setarch -R 并以 -no-pie 构建：受限沙箱/容器里 TSan 对地址布局敏感，
# 偶发 "FATAL: unexpected memory mapping"，这两招是常见的稳定化手段
# （作者沙箱实测：不加时约半数运行 FATAL，加后 4/4 稳定出报告）；
# 在普通开发机上直接运行 TSan 二进制即可，无需 setarch。
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

o=$(timeout 30 "$BIN/bad_01_race")
if echo "$o" | grep -qE "^shared counter = [0-9]+ \(expected 1000000\)$"; then
  ok "bad_01 plain build runs (result unreliable by design)"
else ng "bad_01 plain: $o"; fi

if g++ -std=c++11 -g -no-pie -fsanitize=thread -pthread examples/bad_01_race.cpp -o "$BIN/tsan_race" 2>/dev/null; then
  o=$(timeout 120 setarch "$(uname -m)" -R "$BIN/tsan_race" 2>&1)
  if has "$o" "ThreadSanitizer: data race" && has "$o" "SUMMARY: ThreadSanitizer: data race"; then
    ok "bad_01 caught by TSan (data race reported)"
  else ng "bad_01 TSan report missing: $(echo "$o" | head -5)"; fi
else ng "tsan build of bad_01 failed"; fi

o=$(timeout 5 "$BIN/bad_02_deadlock" 2>&1); rc=$?
if [ $rc -eq 124 ] && has "$o" "thread 1: holding A, waiting for B" && has "$o" "thread 2: holding B, waiting for A"; then
  ok "bad_02 deadlocks as designed (timeout rc=124)"
else ng "bad_02 (rc=$rc out=[$o])"; fi

o=$(timeout 30 "$BIN/ex01_deadlock_fixed")
if has "$o" "A = 1000, B = 1000, total = 2000 (expected)"; then
  ok "ex01 std::lock fixes deadlock, balances conserved"
else ng "ex01: $o"; fi

if g++ -std=c++11 -g -no-pie -fsanitize=thread -pthread examples/ex02_tsan_clean_counter.cpp -o "$BIN/tsan_clean" 2>/dev/null; then
  o=$(timeout 120 setarch "$(uname -m)" -R "$BIN/tsan_clean" 2>&1); rc=$?
  if [ $rc -eq 0 ] && has "$o" "mutex counter = 1000000 (expected 1000000)" && ! has "$o" "ThreadSanitizer"; then
    ok "ex02 TSan-clean, exact result"
  else ng "ex02 tsan (rc=$rc out=[$o])"; fi
else ng "tsan build of ex02 failed"; fi

echo "----"; echo "passed=$pass failed=$fail"
[ $fail -eq 0 ]
