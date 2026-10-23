#!/usr/bin/env bash
# 第一部分第 2 章（指针与内存二）验收测试 —— 2026-10-23 补齐。
# 断言取自正文核对要点；含 bad_01 的 ASan 定向检查。
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

o=$("$BIN/ex01_double_pointer")
if has "$o" "x = 2" && has "$o" "10 20 30 40"; then
  ok "ex01 double pointer"
else ng "ex01: $o"; fi

o=$("$BIN/ex02_heap_stack")
if has "$o" "heap array: 100 101 102" && has "$o" "global" && has "$o" "stack" && has "$o" "heap var"; then
  ok "ex02 memory regions + heap array"
else ng "ex02: $o"; fi

o=$("$BIN/ex03_nullptr")
if has "$o" "f(int) called" && has "$o" "f(int*) called" && has "$o" "p is null, do not dereference it"; then
  ok "ex03 nullptr overloads"
else ng "ex03: $o"; fi

# bad_01：悬垂指针反例——普通构建输出不可信，只验 ASan 构建必须报 heap-use-after-free
tmp=$(mktemp -d)
if g++ -std=c++11 -g -fsanitize=address examples/bad_01_dangling.cpp -o "$tmp/bad_asan" 2>/dev/null; then
  out=$("$tmp/bad_asan" 2>&1); rc=$?
  if [ $rc -ne 0 ] && echo "$out" | grep -q "heap-use-after-free"; then
    ok "bad_01 ASan reports heap-use-after-free"
  else ng "bad_01 ASan (rc=$rc)"; fi
else ng "bad_01 ASan build"; fi
rm -rf "$tmp"

echo "----"; echo "passed=$pass failed=$fail"
[ $fail -eq 0 ]
