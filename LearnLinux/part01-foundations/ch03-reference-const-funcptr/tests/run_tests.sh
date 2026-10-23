#!/usr/bin/env bash
# 第一部分第 3 章（引用、const 与函数指针）验收测试 —— 2026-10-23 补齐。
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

o=$("$BIN/ex01_reference")
if has "$o" "a = 25, r = 25" && has "$o" "a = 99, b = 99"; then ok "ex01 reference alias"; else ng "ex01: $o"; fi

o=$("$BIN/ex02_const_pointer")
if has "$o" "x = 30, y = 2" && has "$o" "*p2 = 2, *p4 = 30"; then ok "ex02 const combos"; else ng "ex02: $o"; fi

o=$("$BIN/ex03_ref_params")
if has "$o" "after swapByPtr: a=2, b=1" && has "$o" "after swapByRef: a=1, b=2" \
   && has "$o" "copies after byValue   = 1" && has "$o" "copies after byConstRef = 1"; then
  ok "ex03 swaps + copy counts"
else ng "ex03: $o"; fi

o=$("$BIN/ex04_func_pointer")
if has "$o" "6 add 3 = 9" && has "$o" "6 sub 3 = 3" && has "$o" "6 mul 3 = 18" \
   && has "$o" "1 2 4 7 9" && has "$o" "9 7 4 2 1"; then
  ok "ex04 dispatch table + callback sort"
else ng "ex04: $o"; fi

echo "----"; echo "passed=$pass failed=$fail"
[ $fail -eq 0 ]
