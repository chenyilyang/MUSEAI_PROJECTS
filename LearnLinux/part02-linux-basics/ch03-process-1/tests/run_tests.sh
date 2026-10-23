#!/usr/bin/env bash
# 第二部分第 3 章（进程一）验收测试 —— 2026-10-23 补齐。
# 断言取自正文核对要点；PID 数值每次不同，只断言关系。ex04 含 ps 实测僵尸，约需 12 秒。
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

o=$("$BIN/ex01_fork_basic")
cnt=$(echo "$o" | grep -c "before fork")
ppid=$(echo "$o" | sed -n 's/.*parent: my pid = \([0-9]*\), my child = \([0-9]*\)/\1/p')
cpid=$(echo "$o" | sed -n 's/.*parent: my pid = \([0-9]*\), my child = \([0-9]*\)/\2/p')
if [ "$cnt" = 1 ] && [ -n "$ppid" ] \
   && has "$o" "child : my pid = $cpid, my parent = $ppid"; then
  ok "ex01 fork identity split (before-fork once, child parent == parent pid)"
else ng "ex01 (cnt=$cnt): $o"; fi

o=$("$BIN/ex02_fork_exec")
nlines=$(echo "$o" | wc -l)
if [ "$nlines" = 2 ] && has "$o" "hello from exec" && has "$o" "parent: child finished" \
   && ! echo "$o" | grep -q "never appears"; then
  ok "ex02 exec replaces image, buffered line vanished"
else ng "ex02: $o"; fi

o=$("$BIN/ex03_wait_status")
if hase "$o" "child [0-9]+: exited = 1, code = 42" && hase "$o" "child [0-9]+: signaled = 1, sig = 15"; then
  ok "ex03 status decode: exit 42 / signal 15"
else ng "ex03: $o"; fi

# ex04：后台运行，3 秒时 ps 抓僵尸，跑完看孤儿新 PPID
outf=$(mktemp)
"$BIN/ex04_zombie_orphan" >"$outf" 2>&1 &
wp=$!
sleep 3
zpid=$(grep -o "child [0-9]*" "$outf" | head -1 | awk '{print $2}')
stat=$(ps -o stat= -p "$zpid" 2>/dev/null || echo "?")
wait $wp
sleep 3   # 孤儿进程在父进程退出后 2 秒才打印，先等它落字再读全文
full=$(cat "$outf")
if echo "$stat" | grep -q "^Z" && has "$full" "reaped child $zpid" \
   && echo "$full" | grep -qE "\[orphan\] my pid = [0-9]+, my new parent = [0-9]+"; then
  ok "ex04 zombie seen as Z in ps, reaped later; orphan adopted (ppid=$(echo "$full" | sed -n 's/.*my new parent = //p'))"
else ng "ex04 (stat=$stat zpid=$zpid): $full"; fi
rm -f "$outf"

echo "----"; echo "passed=$pass failed=$fail"
[ $fail -eq 0 ]
