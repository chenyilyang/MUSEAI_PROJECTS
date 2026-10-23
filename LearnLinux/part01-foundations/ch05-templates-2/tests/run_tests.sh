#!/usr/bin/env bash
# 第一部分第 5 章（模板基础二）验收测试 —— 2026-10-23 补齐。
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

o=$("$BIN/ex01_specialization")
if hase "$o" "^int:\s+generic type$" && has "$o" "double: generic type" && has "$o" "string: C string (const char*)"; then
  ok "ex01 full specialization"
else ng "ex01: $o"; fi

o=$("$BIN/ex02_partial_spec")
if has "$o" "Box<T> generic, get=7" && has "$o" "Box<T*> pointer specialization, deref=8" && has "$o" "x after = 9"; then
  ok "ex02 partial specialization"
else ng "ex02: $o"; fi

o=$("$BIN/ex03_nontype_param")
if has "$o" "FixedArray<int,4> sum = 10" && has "$o" "FixedArray<double,3> sum = 1.500000"; then
  ok "ex03 non-type parameter"
else ng "ex03: $o"; fi

o=$("$BIN/ex04_variadic_print")
if has "$o" "1 2.5 can x" && has "$o" "only 2"; then
  ok "ex04 variadic print"
else ng "ex04: $o"; fi

echo "----"; echo "passed=$pass failed=$fail"
[ $fail -eq 0 ]
