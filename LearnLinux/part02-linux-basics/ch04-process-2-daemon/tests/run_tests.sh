#!/usr/bin/env bash
# 第二部分第 4 章（进程二与守护进程）验收测试 —— 2026-10-23 补齐。
# 断言取自正文核对要点；ex02 走完整生命周期：启动→ps 查身份→日志增长→SIGTERM 收尾。
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

o=$("$BIN/ex01_session_group")
lpgid=$(echo "$o" | sed -n 's/.*launcher: pid = [0-9]*, pgid = \([0-9]*\).*/\1/p')
bpgid=$(echo "$o" | sed -n 's/.*child before setsid: pid = [0-9]*, pgid = \([0-9]*\).*/\1/p')
apid=$(echo "$o" | sed -n 's/.*child after  setsid: pid = \([0-9]*\), pgid = \([0-9]*\), sid = \([0-9]*\)/\1/p')
apgid=$(echo "$o" | sed -n 's/.*child after  setsid: pid = \([0-9]*\), pgid = \([0-9]*\), sid = \([0-9]*\)/\2/p')
asid=$(echo "$o" | sed -n 's/.*child after  setsid: pid = \([0-9]*\), pgid = \([0-9]*\), sid = \([0-9]*\)/\3/p')
if [ -n "$lpgid" ] && [ "$lpgid" = "$bpgid" ] && [ -n "$apid" ] \
   && [ "$apid" = "$apgid" ] && [ "$apgid" = "$asid" ]; then
  ok "ex01 setsid: before shares group, after pid==pgid==sid"
else ng "ex01 (lpgid=$lpgid bpgid=$bpgid after=$apid/$apgid/$asid)"; fi

o=$("$BIN/ex03_environ")
if hase "$o" "^HOME = /" && has "$o" "parent sees LEARNLINUX_DEMO = cockpit" \
   && has "$o" "child inherits LEARNLINUX_DEMO = cockpit" \
   && has "$o" "parent still sees LEARNLINUX_DEMO = cockpit"; then
  ok "ex03 environ inherited snapshot, no write-back"
else ng "ex03: $o"; fi

# ex02 守护进程生命周期（示例内写死 /tmp 路径，串行使用）
LOG=/tmp/learnlinux_daemon.log
PIDF=/tmp/learnlinux_daemon.pid
rm -f "$LOG" "$PIDF"
"$BIN/ex02_daemon"
sleep 0.5
dpid=$(cat "$PIDF" 2>/dev/null || echo "")
if [ -n "$dpid" ] && kill -0 "$dpid" 2>/dev/null; then
  read -r _ppid _pgid _sid _tty <<< "$(ps -o ppid=,pgid=,sid=,tty= -p "$dpid")"
  n1=$(wc -l < "$LOG")
  sleep 2.2
  n2=$(wc -l < "$LOG")
  kill -TERM "$dpid"
  sleep 1.2
  alive=no; kill -0 "$dpid" 2>/dev/null && alive=yes
  last=$(tail -1 "$LOG")
  if [ "$_ppid" = 1 ] && [ "$_tty" = "?" ] && [ "$_pgid" != "$dpid" ] && [ "$_pgid" = "$_sid" ] \
     && [ "$n2" -gt "$n1" ] && [ "$alive" = no ] && [ "$last" = "daemon exiting on SIGTERM" ]; then
    ok "ex02 daemon lifecycle (ppid=1, tty=?, log grows $n1->$n2, SIGTERM farewell)"
  else ng "ex02 daemon (ppid=$_ppid pgid=$_pgid sid=$_sid tty=$_tty n=$n1->$n2 alive=$alive last=[$last])"; fi
else
  ng "ex02 daemon did not start (pidfile=[$dpid])"
fi
rm -f "$LOG" "$PIDF"

echo "----"; echo "passed=$pass failed=$fail"
[ $fail -eq 0 ]
