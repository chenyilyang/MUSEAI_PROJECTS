#!/usr/bin/env bash
# 阶段项目 TaskQueue + Logger 验收脚本 —— 先于实现写定（测试先行）。
#
# 用法：
#   bash tests/run_tests.sh              全流程：参考实现全过 + 双 sanitizer 干净；骨架必须不过
#   bash tests/run_tests.sh <构建目录>   只测该目录下的三个验收程序（须已构建）
#
# 断言面（与项目 README 的输出契约一一对应）：
#   queue_check   无界队列 FIFO + close 后排空、pop 收到结束信号；有界队列满时 push 真阻塞
#   logger_stress 4 生产者 × 2500 条并发写日志：落盘 8000 行（DEBUG 被级别过滤），
#                 逐行格式完整不交错、级别计数精确、每条消息恰好出现一次（不丢尾、不重复）
#   pool_check    线程池结果精确；shutdown 排空语义：关后计数必须满额
# TSan 构建在本脚本里以 -no-pie + setarch -R 运行（受限沙箱的稳定化手段，同第 5 章说明）；
# 普通开发机直接运行 TSan 二进制即可。
set -u
cd "$(dirname "$0")/.."
ROOT=$(pwd)
pass=0; fail=0
ok() { echo "PASS: $1"; pass=$((pass+1)); }
ng() { echo "FAIL: $1"; fail=$((fail+1)); }
has() { echo "$1" | grep -qF "$2"; }

suite() {  # $1=二进制目录 $2=标签
  local dir="$1" label="$2"
  local log="$ROOT/.acc_stress.log"
  rm -f "$log"

  local o rc
  o=$(timeout 20 "$dir/queue_check" 2>&1); rc=$?
  if [ $rc -eq 0 ] && has "$o" "unbounded: count = 100, sum = 5050" \
     && has "$o" "unbounded: pop returns false after drain" \
     && has "$o" "bounded: third push blocked while full" \
     && has "$o" "bounded: order preserved 1 2 3"; then
    ok "$label: queue_check"
  else ng "$label: queue_check (rc=$rc out=[$o])"; fi

  o=$(timeout 60 "$dir/logger_stress" "$log" 4 2500 2>&1); rc=$?
  local vout vrc
  vout=$(python3 "$ROOT/tests/verify_log.py" "$log" 4 2500 2>&1); vrc=$?
  if [ $rc -eq 0 ] && [ $vrc -eq 0 ] && has "$o" "stress done: producers = 4, per = 2500"; then
    ok "$label: logger_stress ($vout)"
  else ng "$label: logger_stress (rc=$rc verify=[$vout] out=[$o])"; fi
  rm -f "$log"

  o=$(timeout 20 "$dir/pool_check" 2>&1); rc=$?
  if [ $rc -eq 0 ] && has "$o" "pool: sum = 2470 (expected 2470)" \
     && has "$o" "pool: drained 10/10 before shutdown returned"; then
    ok "$label: pool_check"
  else ng "$label: pool_check (rc=$rc out=[$o])"; fi
}

build() {  # $1=源码目录 $2=构建目录
  cmake -S "$1" -B "$2" >/dev/null 2>&1 && cmake --build "$2" >/dev/null 2>&1
}

sanitize_run() {  # $1=标签 $2...=编译参数；对 solutions 三件套各编一份并跑小规模
  local label="$1"; shift
  local d="$ROOT/solutions/build-$label"
  rm -rf "$d"; mkdir -p "$d"
  local all_ok=1 o rc
  local pre=""
  if [ "$label" = tsan ] && command -v setarch >/dev/null 2>&1; then
    pre="setarch $(uname -m) -R"   # 受限沙箱里 TSan 需要它稳定地址布局；普通机无此命令时直跑
  fi
  for p in queue_check logger_stress pool_check; do
    if ! g++ -std=c++11 -g -Wall -Wextra "$@" -pthread \
         "$ROOT/solutions/$p.cpp" -o "$d/$p" 2>/dev/null; then
      all_ok=0; break
    fi
  done
  if [ $all_ok -eq 1 ]; then
    o=$(timeout 60 $pre "$d/queue_check" 2>&1); rc=$?
    [ $rc -eq 0 ] && ! echo "$o" | grep -qi "sanitizer" || all_ok=0
    o=$(timeout 60 $pre "$d/logger_stress" "$d/s.log" 2 500 2>&1); rc=$?
    [ $rc -eq 0 ] && ! echo "$o" | grep -qi "sanitizer" || all_ok=0
    python3 "$ROOT/tests/verify_log.py" "$d/s.log" 2 500 >/dev/null 2>&1 || all_ok=0
    o=$(timeout 60 $pre "$d/pool_check" 2>&1); rc=$?
    [ $rc -eq 0 ] && ! echo "$o" | grep -qi "sanitizer" || all_ok=0
  fi
  rm -rf "$d"
  if [ $all_ok -eq 1 ]; then ok "solutions $label clean"; else ng "solutions $label"; fi
}

if [ $# -ge 1 ]; then
  suite "$1" "custom"
  echo "----"; echo "passed=$pass failed=$fail"
  [ $fail -eq 0 ]
  exit $?
fi

echo "== reference implementation must pass =="
if build solutions solutions/build; then
  suite "$ROOT/solutions/build" "solutions"
else
  ng "solutions build"
fi

echo "== reference under ASan+UBSan must be clean =="
sanitize_run asan -fsanitize=address,undefined

echo "== reference under TSan must be clean =="
sanitize_run tsan -no-pie -fsanitize=thread

echo "== starter skeleton must FAIL =="
if build starter starter/build; then
  before_fail=$fail
  suite "$ROOT/starter/build" "starter"
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
