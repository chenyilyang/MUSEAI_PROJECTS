#!/usr/bin/env bash
# 第 5 章（信号）示例验收测试 —— 先于示例实现写定（测试先行）。
# 用法：在章节目录下执行 bash tests/run_tests.sh
# 做什么：构建 examples/，然后逐项驱动三个示例，断言它们的可观察行为：
#   ex01 两种装法（signal / sigaction）下，进程挨第一下 SIGINT 不死，
#        挨满两下后打印 interrupts = 2 并以 0 退出；
#   ex02 5 个子进程被 SIGCHLD 驱动的 WNOHANG 循环全部回收（退出码 11..15），
#        且最后以 ECHILD 证明没有僵尸残留；
#   ex03 500ms 定时器下在 2、4 次时打印摘要，总数恰好 6 次后收尾。
set -u
cd "$(dirname "$0")/.."
BIN="examples/build"

pass=0; fail=0
ok() { echo "PASS: $1"; pass=$((pass+1)); }
ng() { echo "FAIL: $1"; fail=$((fail+1)); }

echo "== build examples =="
if cmake -S examples -B "$BIN" >/dev/null 2>&1 && cmake --build "$BIN" >/dev/null 2>&1; then
  ok "examples build"
else
  ng "examples build (cmake or compile failed)"
  echo "----"; echo "passed=$pass failed=$fail"
  exit 1
fi

echo "== ex01: SIGINT counted, survives the first, exits after the second =="
for mode in signal sigaction; do
  out=$(mktemp)
  "$BIN/ex01_signal_vs_sigaction" "$mode" >"$out" 2>&1 &
  p=$!
  sleep 0.5; kill -INT $p; sleep 0.5
  if kill -0 $p 2>/dev/null; then alive=yes; else alive=no; fi
  kill -INT $p 2>/dev/null
  wait $p; rc=$?
  if [ "$alive" = yes ] && [ $rc -eq 0 ] \
     && grep -q "interrupts = 2" "$out" && grep -q "mode = $mode" "$out"; then
    ok "ex01 mode=$mode"
  else
    ng "ex01 mode=$mode (alive=$alive rc=$rc out=$(cat "$out"))"
  fi
  rm -f "$out"
done

echo "== ex02: SIGCHLD-driven reaping, 5 children, no zombies =="
out=$(mktemp)
timeout 15 "$BIN/ex02_sigchld_reap" >"$out" 2>&1; rc=$?
n=$(grep -c "^reaped child" "$out")
codes=$(grep "^reaped child" "$out" | sed -E 's/.*exit = ([0-9]+).*/\1/' | sort -n | tr '\n' ' ')
if [ $rc -eq 0 ] && [ "$n" = 5 ] && [ "$codes" = "11 12 13 14 15 " ] \
   && grep -q "no zombies left (ECHILD)" "$out"; then
  ok "ex02 reaps all 5, exit codes 11..15, ECHILD proof"
else
  ng "ex02 (rc=$rc reaped=$n codes=[$codes])"; cat "$out"
fi
rm -f "$out"

echo "== ex03: 500ms timer ticks =="
out=$(mktemp)
timeout 15 "$BIN/ex03_timer_tick" >"$out" 2>&1; rc=$?
if [ $rc -eq 0 ] && grep -q "ticks = 2" "$out" && grep -q "ticks = 4" "$out" \
   && grep -q "total ticks = 6" "$out"; then
  ok "ex03 timer: summaries at 2/4, total 6"
else
  ng "ex03 (rc=$rc)"; cat "$out"
fi
rm -f "$out"

echo "----"
echo "passed=$pass failed=$fail"
[ $fail -eq 0 ]
