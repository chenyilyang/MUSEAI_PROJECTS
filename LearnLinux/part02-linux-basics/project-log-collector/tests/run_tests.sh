#!/usr/bin/env bash
# 阶段项目 Log Collector 验收脚本 —— 先于实现写定（测试先行）。
#
# 用法：
#   bash tests/run_tests.sh              全流程：参考实现须全过；骨架须不过；ASan 复跑须干净
#   bash tests/run_tests.sh <二进制>    只测指定的某个实现
#
# 断言面（与项目 README 的输出契约一一对应）：
#   A 正常跑：log_collector log 3 10 退出码 0；日志内容行恰 30 行且全员格式正确；
#     每个 worker 的 seq 1..10 各一次；末行是收尾行；stdout 三行齐全。
#   B 优雅关闭：大任务中途发 SIGTERM，进程须在时限内以 0 退出，日志末行是收尾行，
#     且系统里不许残留同名进程（worker 没被丢下）。
set -u
cd "$(dirname "$0")/.."
ROOT=$(pwd)

pass=0; fail=0
ok() { echo "PASS: $1"; pass=$((pass+1)); }
ng() { echo "FAIL: $1"; fail=$((fail+1)); }

TSRE='^\[[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}\] '

suite() {
  local bin="$1" label="$2"
  local log="$ROOT/.acc_basic.log" log2="$ROOT/.acc_term.log"
  rm -f "$log" "$log2"

  # ---- A: 正常跑 ----
  local out rc
  out=$(timeout 20 "$bin" "$log" 3 10 2>&1); rc=$?
  local nlines badfmt
  nlines=$(grep -cE "${TSRE}worker-[0-9]+ msg [0-9]+$" "$log" 2>/dev/null || true)
  badfmt=$(grep -cvE "${TSRE}(worker-[0-9]+ msg [0-9]+|collector shutdown: workers=3 lines=30)$" "$log" 2>/dev/null || true)
  local a_ok=1
  [ $rc -eq 0 ] || a_ok=0
  [ "$nlines" = 30 ] || a_ok=0
  [ "$badfmt" = 0 ] || a_ok=0
  for w in 0 1 2; do
    local got
    got=$(grep -E "] worker-$w msg " "$log" | sed -E 's/.*msg //' | sort -n | tr '\n' ' ')
    [ "$got" = "1 2 3 4 5 6 7 8 9 10 " ] || a_ok=0
  done
  tail -1 "$log" | grep -qE "${TSRE}collector shutdown: workers=3 lines=30$" || a_ok=0
  echo "$out" | grep -q "^total lines = 30$" || a_ok=0
  echo "$out" | grep -q "^worker exit codes: 0 1 2$" || a_ok=0
  echo "$out" | grep -q "^reaped 3 workers, no zombies (ECHILD)$" || a_ok=0
  if [ $a_ok -eq 1 ]; then ok "$label: basic run (A)"; else
    ng "$label: basic run (A) rc=$rc lines=$nlines badfmt=$badfmt out=[$out]"
  fi

  # ---- B: SIGTERM 优雅关闭 ----
  "$bin" "$log2" 3 100000 >/dev/null 2>&1 &
  local p=$!
  local waited=0
  while [ $waited -lt 50 ]; do
    [ -f "$log2" ] && [ "$(wc -l < "$log2" 2>/dev/null || echo 0)" -ge 5 ] && break
    sleep 0.1; waited=$((waited+1))
  done
  kill -TERM $p 2>/dev/null
  local tries=0
  while kill -0 $p 2>/dev/null && [ $tries -lt 100 ]; do sleep 0.1; tries=$((tries+1)); done
  if kill -0 $p 2>/dev/null; then kill -9 $p 2>/dev/null; fi
  wait $p; rc=$?
  sleep 0.5
  local leftovers
  leftovers=$(pgrep -x log_collector | wc -l)
  local b_ok=1
  [ $rc -eq 0 ] || b_ok=0
  [ "$leftovers" = 0 ] || b_ok=0
  tail -1 "$log2" 2>/dev/null | grep -qE "${TSRE}collector shutdown: workers=3 lines=[0-9]+$" || b_ok=0
  if [ $b_ok -eq 1 ]; then ok "$label: graceful SIGTERM (B)"; else
    ng "$label: graceful SIGTERM (B) rc=$rc leftovers=$leftovers lastlog=[$(tail -1 "$log2" 2>/dev/null)]"
  fi
  rm -f "$log" "$log2"
}

build() {  # $1=源码目录 $2=构建目录
  cmake -S "$1" -B "$2" >/dev/null 2>&1 && cmake --build "$2" >/dev/null 2>&1
}

if [ $# -ge 1 ]; then
  suite "$1" "custom"
  echo "----"; echo "passed=$pass failed=$fail"
  [ $fail -eq 0 ]
  exit $?
fi

echo "== reference implementation must pass =="
if build solutions solutions/build; then
  suite "$ROOT/solutions/build/log_collector" "solutions"
else
  ng "solutions build"
fi

echo "== reference under ASan+UBSan must be clean =="
if cmake -S solutions -B solutions/build-asan \
      -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -g" >/dev/null 2>&1 \
   && cmake --build solutions/build-asan >/dev/null 2>&1; then
  asan_out=$(timeout 20 "$ROOT/solutions/build-asan/log_collector" "$ROOT/.acc_asan.log" 2 5 2>&1); asan_rc=$?
  if [ $asan_rc -eq 0 ] && ! echo "$asan_out" | grep -qi "sanitizer" \
     && echo "$asan_out" | grep -q "^total lines = 10$"; then
    ok "solutions ASan/UBSan run clean"
  else
    ng "solutions ASan/UBSan run (rc=$asan_rc out=[$asan_out])"
  fi
  rm -f "$ROOT/.acc_asan.log"
else
  ng "solutions ASan build"
fi

echo "== starter skeleton must FAIL =="
if build starter starter/build; then
  before_fail=$fail
  suite "$ROOT/starter/build/log_collector" "starter"
  if [ $fail -gt $before_fail ]; then
    ok "starter fails acceptance as designed"
    fail=$before_fail   # 骨架不过是预期结果，不计入总失败
  else
    ng "starter unexpectedly PASSES acceptance (skeleton too complete?)"
  fi
else
  ng "starter build (skeleton must at least compile)"
fi

echo "----"; echo "passed=$pass failed=$fail"
[ $fail -eq 0 ]
