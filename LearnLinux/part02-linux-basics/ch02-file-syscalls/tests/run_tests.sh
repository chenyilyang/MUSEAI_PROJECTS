#!/usr/bin/env bash
# 第二部分第 2 章（文件与系统调用）验收测试 —— 2026-10-23 补齐。
# 断言取自正文核对要点；ex02 的 errno 只断言"不再是 2"，不断言具体数值（正文已注明）。
set -u
cd "$(dirname "$0")/.."
BIN="$PWD/examples/build"
pass=0; fail=0
ok() { echo "PASS: $1"; pass=$((pass+1)); }
ng() { echo "FAIL: $1"; fail=$((fail+1)); }
has() { echo "$1" | grep -qF "$2"; }

if cmake -S examples -B examples/build >/dev/null 2>&1 && cmake --build examples/build >/dev/null 2>&1; then
  ok "build"
else ng "build"; echo "passed=$pass failed=$fail"; exit 1; fi

T=$(mktemp -d)
o=$(cd "$T" && "$BIN/ex01_open_write_read")
if has "$o" "opened fd = 3" && has "$o" "wrote 15 bytes" && has "$o" "reopened fd = 3" \
   && has "$o" "read 15 bytes: hello, syscall"; then
  ok "ex01 fd=3 twice, 15 bytes round-trip"
else ng "ex01: $o"; fi

out=$(cd "$T" && "$BIN/ex02_errno" 2>/tmp/ch02_ex02_err.txt)
err=$(cat /tmp/ch02_ex02_err.txt)
now=$(echo "$out" | sed -n 's/.*errno now = \([0-9]*\).*/\1/p')
if has "$out" "open failed, errno = 2 (ENOENT = 2)" && has "$out" "strerror: No such file or directory" \
   && has "$out" "successful open, fd = 3" && [ -n "$now" ] && [ "$now" != "2" ] \
   && has "$err" "perror says: No such file or directory"; then
  ok "ex02 errno discipline (now=$now, no longer 2)"
else ng "ex02 (now=[$now] out=[$out] err=[$err])"; fi

o=$(cd "$T" && "$BIN/ex03_lseek")
if has "$o" "from offset 10: ABCDEF" && has "$o" "file size = 16" \
   && has "$o" "last 4 bytes: CDEF" && has "$o" "lseek on pipe: ret = -1, errno = 29 (ESPIPE = 29)"; then
  ok "ex03 lseek reads + ESPIPE on pipe"
else ng "ex03: $o"; fi

o=$(cd "$T" && "$BIN/ex04_nonblock")
if has "$o" "empty nonblock read: n = -1, errno = 11 (EAGAIN = 11)" \
   && has "$o" "after write: n = 4, data = ping"; then
  ok "ex04 nonblock EAGAIN then ping"
else ng "ex04: $o"; fi

rm -rf "$T"
echo "----"; echo "passed=$pass failed=$fail"
[ $fail -eq 0 ]
