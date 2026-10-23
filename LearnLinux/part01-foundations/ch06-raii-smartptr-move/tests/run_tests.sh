#!/usr/bin/env bash
# 第一部分第 6 章（RAII、智能指针与移动语义）验收测试 —— 2026-10-23 补齐。
# 断言取自正文实测计数（libstdc++ 环境）：计数序列、循环引用泄漏对照、vector 扩容 moves=6。
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

# ex01 会在当前目录生成文件，进临时目录跑
tmp=$(mktemp -d)
ABS="$PWD/$BIN/ex01_raii_file"
o=$(cd "$tmp" && "$ABS")
if [ "$(echo "$o" | grep -c "closed automatically")" = 2 ] \
   && has "$o" "content of ex01_normal.txt: hello raii" && has "$o" "content of ex01_early.txt: early return"; then
  ok "ex01 RAII auto-close on both paths"
else ng "ex01: $o"; fi
rm -rf "$tmp"

o=$("$BIN/ex02_unique_ptr"); rc=$?
l2=$(echo "$o" | grep -n "Sensor 2 destroyed" | cut -d: -f1)
l1=$(echo "$o" | grep -n "Sensor 1 destroyed" | cut -d: -f1)
if [ $rc -eq 0 ] && has "$o" "after move: p1 empty, p2 owns Sensor 1" \
   && [ -n "$l2" ] && [ -n "$l1" ] && [ "$l2" -lt "$l1" ]; then
  ok "ex02 unique_ptr move + reverse destruction"
else ng "ex02: $o"; fi

o=$("$BIN/ex03_shared_weak")
if has "$o" "count after create: 1" && has "$o" "count after copy:   2" \
   && has "$o" "count after copy2:  3" && has "$o" "count after scope:  1" \
   && has "$o" "NodeShared alive after scope: 2 (leaked!)" && has "$o" "NodeWeak alive after scope:   0 (clean)"; then
  ok "ex03 shared counts 1-2-3-1, cycle leaks vs weak clean"
else ng "ex03: $o"; fi

o=$("$BIN/ex04_move_semantics")
if has "$o" "copies=1 moves=0 (expect 1, 0)" && has "$o" "copies=1 moves=1 (expect 1, 1); a.size=0 (moved-from)" \
   && has "$o" "vector growth: copies=0 moves=6 (expect 0, 6)"; then
  ok "ex04 move counters incl. vector growth 0/6"
else ng "ex04: $o"; fi

echo "----"; echo "passed=$pass failed=$fail"
[ $fail -eq 0 ]
