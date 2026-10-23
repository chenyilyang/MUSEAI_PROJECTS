#!/usr/bin/env bash
# 第一部分第 4 章（模板基础一）验收测试 —— 2026-10-23 补齐。
set -u
cd "$(dirname "$0")/.."
BIN="examples/build"
pass=0; fail=0
ok() { echo "PASS: $1"; pass=$((pass+1)); }
ng() { echo "FAIL: $1"; fail=$((fail+1)); }
has() { echo "$1" | grep -qF "$2"; }
hase() { echo "$1" | grep -qE "$2"; }
cnt() { echo "$1" | grep -cF "$2"; }

if cmake -S examples -B "$BIN" >/dev/null 2>&1 && cmake --build "$BIN" >/dev/null 2>&1; then
  ok "build"
else ng "build"; echo "passed=$pass failed=$fail"; exit 1; fi

o=$("$BIN/ex01_func_template")
if hase "$o" "^int:\s+7$" && has "$o" "double: 2.500000" && has "$o" "string: dds" && has "$o" "mixed:  7.500000"; then
  ok "ex01 function template basics"
else ng "ex01: $o"; fi

o=$("$BIN/ex02_deduction")
if has "$o" "byVal : T=Pi   sizeof(T)=8" && has "$o" "byVal : T=PKc  sizeof(T)=8" \
   && [ "$(cnt "$o" "byVal : T=i    sizeof(T)=4")" = 2 ] \
   && [ "$(cnt "$o" "byRef : sizeof(param)=4")" = 2 ] \
   && [ "$(cnt "$o" "byRef : sizeof(param)=16")" = 1 ]; then
  ok "ex02 deduction byVal/byRef"
else ng "ex02: $o"; fi

o=$("$BIN/ex03_class_template")
if has "$o" "(2500, rpm)" && has "$o" "Box<int> = 42" && has "$o" "Box<string> = SOME/IP"; then
  ok "ex03 class template"
else ng "ex03: $o"; fi

o=$("$BIN/ex04_overload_with_template")
if has "$o" "ordinary function (int), value=5" && has "$o" "template function, T=d" \
   && has "$o" "template function, T=PKc" && has "$o" "template function, T=i"; then
  ok "ex04 overload vs template choice"
else ng "ex04: $o"; fi

echo "----"; echo "passed=$pass failed=$fail"
[ $fail -eq 0 ]
