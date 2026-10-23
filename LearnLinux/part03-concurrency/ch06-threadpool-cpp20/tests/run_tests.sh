#!/usr/bin/env bash
# 第三部分第 6 章（线程池 + C++20 并发专题）验收测试 —— 先于示例实现写定（测试先行）。
# 断言面：
#   ex01（C++11）线程池：20 个平方任务经 future 按序取回、结果与总和精确；
#         立即关闭演示：只跑完在执行中的 1 个，队列里 5 个任务的 future 全部 broken；
#         另以 TSan 构建复核池实现零报告（本部分的硬性要求）。
#   ex02（C++20）jthread + stop_token：请求停止后限时内自行退出，无手动 join。
#   ex03（C++20）semaphore 限流实测最大并发恰为 3；barrier 使两阶段日志严格分段。
#   ex04（C++20）生成器协程：斐波那契前 10 项精确。
#   solutions：练习 2（BlockingQueue 作池队列）平方和精确；练习 3（jthread 重写）参数与自动 join 行齐全。
# TSan 说明同第 5 章：本脚本以 -no-pie 构建、setarch -R 运行以避开受限沙箱里的
# 地址布局偶发 FATAL；普通开发机直接运行即可。构建以 -Wall -Wextra -Werror 把关。
set -u
cd "$(dirname "$0")/.."
BIN="examples/build"
SBIN="solutions/build"
pass=0; fail=0
ok() { echo "PASS: $1"; pass=$((pass+1)); }
ng() { echo "FAIL: $1"; fail=$((fail+1)); }
has() { echo "$1" | grep -qF "$2"; }

if cmake -S examples -B "$BIN" >/dev/null 2>&1 && cmake --build "$BIN" >/dev/null 2>&1; then
  ok "build examples"
else ng "build examples"; echo "passed=$pass failed=$fail"; exit 1; fi

if cmake -S solutions -B "$SBIN" >/dev/null 2>&1 && cmake --build "$SBIN" >/dev/null 2>&1; then
  ok "build solutions"
else ng "build solutions"; echo "passed=$pass failed=$fail"; exit 1; fi

o=$(timeout 30 "$BIN/ex01_thread_pool"); rc=$?
if [ $rc -eq 0 ] && has "$o" "results: 0 1 4 9 16 25 36 49 64 81 100 121 144 169 196 225 256 289 324 361" \
   && has "$o" "sum of squares 0..19 = 2470 (expected 2470)"; then
  ok "ex01 pool results exact via futures"
else ng "ex01 results (rc=$rc out=[$o])"; fi

if has "$o" "shutdownNow demo: ran = 1, broken futures = 5 (expected 1 and 5)"; then
  ok "ex01 shutdownNow discards queued tasks (broken promises)"
else ng "ex01 shutdownNow: [$o]"; fi

if g++ -std=c++11 -g -no-pie -fsanitize=thread -pthread examples/ex01_thread_pool.cpp -o "$BIN/tsan_pool" 2>/dev/null; then
  o=$(timeout 120 setarch "$(uname -m)" -R "$BIN/tsan_pool" 2>&1); rc=$?
  if [ $rc -eq 0 ] && has "$o" "sum of squares 0..19 = 2470 (expected 2470)" && ! has "$o" "ThreadSanitizer"; then
    ok "ex01 TSan-clean"
  else ng "ex01 tsan (rc=$rc out=[$(echo "$o" | head -5)])"; fi
else ng "tsan build of ex01 failed"; fi

o=$(timeout 10 "$BIN/ex02_jthread"); rc=$?
if [ $rc -eq 0 ] && echo "$o" | grep -qE "worker: stop requested, exiting at tick [0-9]+" \
   && has "$o" "main: jthread auto-joined on scope exit"; then
  ok "ex02 jthread stops cooperatively, auto-joins"
else ng "ex02 (rc=$rc out=[$o])"; fi

o=$(timeout 30 "$BIN/ex03_sem_latch_barrier"); rc=$?
seg=$(echo "$o" | grep -E "phase [01] work|phase boundary" | awk '
  /phase 0 work/ { if (st != 0) bad=1; c0++ }
  /phase boundary/ { if (st == 0) st=1; else if (st == 2) st=3; else bad=1 }
  /phase 1 work/ { if (st == 1) st=2; else if (st != 2) bad=1; c1++ }
  END { if (!bad && c0 == 6 && c1 == 6 && st == 3) print "SEGOK" }')
if [ $rc -eq 0 ] && has "$o" "max concurrent = 3 (limit 3)" && [ "$seg" = "SEGOK" ]; then
  ok "ex03 semaphore caps concurrency at 3, barrier segments phases"
else ng "ex03 (rc=$rc seg=[$seg] out=[$o])"; fi

o=$(timeout 10 "$BIN/ex04_coroutine_peek"); rc=$?
if [ $rc -eq 0 ] && has "$o" "fib: 0 1 1 2 3 5 8 13 21 34"; then
  ok "ex04 coroutine generator yields exact fibonacci"
else ng "ex04 (rc=$rc out=[$o])"; fi

o=$(timeout 30 "$SBIN/ex02_pool_on_blocking_queue"); rc=$?
if [ $rc -eq 0 ] && has "$o" "bq pool: sum = 2470 (expected 2470)"; then
  ok "solution ex02 pool on BlockingQueue exact"
else ng "solution ex02 (rc=$rc out=[$o])"; fi

o=$(timeout 10 "$SBIN/ex03_jthread_lifecycle"); rc=$?
if [ $rc -eq 0 ] && has "$o" "params: x = 1, y = 99, z = 7" && has "$o" "main: scope ended, jthread joined automatically"; then
  ok "solution ex03 jthread lifecycle rewrite"
else ng "solution ex03 (rc=$rc out=[$o])"; fi

echo "----"; echo "passed=$pass failed=$fail"
[ $fail -eq 0 ]
