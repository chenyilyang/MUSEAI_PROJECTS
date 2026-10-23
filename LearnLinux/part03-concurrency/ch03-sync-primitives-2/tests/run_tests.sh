#!/usr/bin/env bash
# 第三部分第 3 章（同步原语二）验收测试 —— 先于示例实现写定（测试先行）。
# 断言面：
#   ex01 条件变量交接的日志顺序逐字严格（等待靠谓词，不靠运气）；
#   ex02 有界队列：生产/消费总量与总和精确，水位从未越过容量；
#   ex03 POSIX 计数信号量：6 个任务全部完成，并发占用从未超过许可数 3；
#   bad_01 无谓词裸 wait + 抢先 notify → 丢失唤醒，程序必须挂死（timeout 124 判过）。
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

o=$(timeout 15 "$BIN/ex01_condvar_handoff")
expected="worker: ready
main: worker is ready, giving go
worker: go received
main: done"
if [ "$o" = "$expected" ]; then
  ok "ex01 handoff order exact"
else ng "ex01: [$o]"; fi

o=$(timeout 30 "$BIN/ex02_bounded_queue")
if has "$o" "produced = 1000, consumed = 1000" && has "$o" "consumed sum = 750500 (expected 750500)" \
   && echo "$o" | grep -qE "^max queue size = [0-4] \(capacity 4\)$"; then
  ok "ex02 totals conserved, capacity respected"
else ng "ex02: $o"; fi

o=$(timeout 30 "$BIN/ex03_posix_sem")
mc=$(echo "$o" | sed -n 's/^max concurrent = \([0-9]*\) (limit 3)$/\1/p')
if has "$o" "completed = 6" && [ -n "$mc" ] && [ "$mc" -le 3 ] && [ "$mc" -ge 1 ]; then
  ok "ex03 semaphore caps concurrency (max=$mc), all tasks done"
else ng "ex03: $o"; fi

o=$(timeout 3 "$BIN/bad_01_lost_wakeup" 2>&1); rc=$?
if [ $rc -eq 124 ] && has "$o" "main: notified before anyone waited"; then
  ok "bad_01 hangs as designed (lost wakeup, timeout rc=124)"
else ng "bad_01 (rc=$rc out=[$o])"; fi

echo "----"; echo "passed=$pass failed=$fail"
[ $fail -eq 0 ]
