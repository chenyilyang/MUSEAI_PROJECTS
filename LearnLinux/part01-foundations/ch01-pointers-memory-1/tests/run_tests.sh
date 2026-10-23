#!/usr/bin/env bash
# 第一部分第 1 章（指针与内存一）验收测试 —— 2026-10-23 补齐。
# 断言取自正文核对要点；地址数值每次运行不同，只断言关系与固定值。
set -u
cd "$(dirname "$0")/.."
BIN="examples/build"
pass=0; fail=0
ok() { echo "PASS: $1"; pass=$((pass+1)); }
ng() { echo "FAIL: $1"; fail=$((fail+1)); }
has() { echo "$1" | grep -qF "$2"; }
hase() { echo "$1" | grep -qE "$2"; }

if cmake -S examples -B "$BIN" >/dev/null 2>&1 && cmake --build "$BIN" >/dev/null 2>&1; then
  ok "build"
else ng "build"; echo "passed=$pass failed=$fail"; exit 1; fi

o=$("$BIN/ex01_address")
a=$(echo "$o" | sed -n 's/.*pa holds address: //p')
b=$(echo "$o" | sed -n 's/.*pb holds address: //p')
c=$(echo "$o" | sed -n 's/.*pc holds address: //p')
if has "$o" "value=42" && hase "$o" "sizeof\(int\*\)\s+= 8" && hase "$o" "sizeof\(double\*\)\s+= 8" \
   && hase "$o" "sizeof\(char\*\)\s+= 8" && [ -n "$a" ] && [ "$a" != "$b" ] && [ "$b" != "$c" ] && [ "$a" != "$c" ]; then
  ok "ex01 addresses distinct, sizeof(ptr)=8"
else ng "ex01: $o"; fi

o=$("$BIN/ex02_deref")
if has "$o" "before: score = 60" && has "$o" "read via *p: 60" && has "$o" "after:  score = 95"; then
  ok "ex02 deref sequence 60/60/95"
else ng "ex02: $o"; fi

o=$("$BIN/ex03_pointer_and_array")
first=$(echo "$o" | sed -n '1s/.*= //p')
second=$(echo "$o" | sed -n '2s/.*= //p')
if [ "$first" = "$second" ] && has "$o" "(&arr[4] - &arr[0]) = 4 elements" \
   && has "$o" "arr[2] = 30, *(arr + 2) = 30"; then
  ok "ex03 arr==&arr[0], diff=4, arr[2]=30"
else ng "ex03: $o"; fi

o=$("$BIN/ex04_pointer_arith")
if has "$o" "sum = 64" && has "$o" "(advanced by 4 bytes)"; then
  ok "ex04 sum=64, step=4 bytes"
else ng "ex04: $o"; fi

echo "----"; echo "passed=$pass failed=$fail"
[ $fail -eq 0 ]
