#!/usr/bin/env bash
# 第三部分第 1 章（线程模型）验收测试 —— 先于示例实现写定（测试先行）。
# 断言面：
#   ex01 4 个 worker 分槽求和精确（55/210/465/820），thread id 两两不同且不同于主线程；
#   ex02 值参改不动原值、std::ref 与指针参改得动；
#   ex03 joinable 1→0、detach 线程独立完成；
#   bad_01 未 join 即析构 → SIGABRT 且出现 terminate called；
#   ex04 pthread_join 取回结果 42；
#   ex05 独立计数总和精确 1000000，共享计数不超过 1000000（丢失更新只断言上界）。
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

o=$(timeout 15 "$BIN/ex01_thread_basics")
mainid=$(echo "$o" | sed -n 's/^main thread id = //p')
wids=$(echo "$o" | sed -n 's/^worker [0-9] thread id = //p')
uniq=$(echo "$wids" | sort -u | wc -l)
notinmain=$(echo "$wids" | grep -vc "^$mainid$")
if has "$o" "slot 0 = 55" && has "$o" "slot 1 = 210" && has "$o" "slot 2 = 465" && has "$o" "slot 3 = 820" \
   && [ "$(echo "$wids" | wc -l)" = 4 ] && [ "$uniq" = 4 ] && [ "$notinmain" = 4 ]; then
  ok "ex01 slots exact, 4 distinct worker ids != main id"
else ng "ex01: $o"; fi

o=$(timeout 15 "$BIN/ex02_args_and_ref")
if has "$o" "after value-arg thread: x = 1" && has "$o" "after ref-arg thread: y = 99" \
   && has "$o" "after pointer-arg thread: z = 7"; then
  ok "ex02 value/ref/pointer arg semantics"
else ng "ex02: $o"; fi

o=$(timeout 15 "$BIN/ex03_lifecycle")
if has "$o" "before join: joinable = 1" && has "$o" "after join: joinable = 0" \
   && has "$o" "detached counter = 1"; then
  ok "ex03 joinable transitions + detached completes"
else ng "ex03: $o"; fi

o=$(timeout 15 "$BIN/bad_01_forgot_join" 2>&1); rc=$?
if [ $rc -ne 0 ] && echo "$o" | grep -q "terminate called"; then
  ok "bad_01 aborts with terminate called (rc=$rc)"
else ng "bad_01 (rc=$rc out=[$o])"; fi

o=$(timeout 15 "$BIN/ex04_pthread")
if has "$o" "pthread result = 42"; then
  ok "ex04 pthread join returns 42"
else ng "ex04: $o"; fi

o=$(timeout 30 "$BIN/ex05_race_glimpse")
shared=$(echo "$o" | sed -n 's/^shared counter = \([0-9]*\).*/\1/p')
if has "$o" "separate total = 1000000" && [ -n "$shared" ] && [ "$shared" -le 1000000 ]; then
  ok "ex05 separate total exact; shared=$shared <= 1000000"
else ng "ex05: $o"; fi

echo "----"; echo "passed=$pass failed=$fail"
[ $fail -eq 0 ]
