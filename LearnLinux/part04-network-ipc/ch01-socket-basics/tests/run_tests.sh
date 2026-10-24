#!/usr/bin/env bash
# 第四部分第 1 章（Socket 基础）验收测试 —— 先于示例实现写定（测试先行）。
# 断言面：
#   ex01 字节序：网络字节恒为 12 34 56 78；往返恒等式成立；主机字节序行存在；
#   ex02 TCP 回显：客户端收到完整回显 "hello socket"；另以 TSan 构建复核零报告
#         （端口经值传递、收发双方无共享可变状态，预期干净）；
#   ex03 send_all/recv_all：100000 字节模式数据，字节数与校验和双精确；
#   solutions：练习 2 多轮回显 3/3；练习 3 UDP 回显精确。
# 本章所有联网演示只走回环（127.0.0.1），端口一律动态取得（bind port 0 + getsockname）。
# TSan 说明同前几章：-no-pie 构建、setarch -R 运行以避开受限沙箱的地址布局偶发 FATAL。
set -u
cd "$(dirname "$0")/.."
BIN="examples/build"
SBIN="solutions/build"
pass=0; fail=0
ok() { echo "PASS: $1"; pass=$((pass+1)); }
ng() { echo "FAIL: $1"; fail=$((fail+1)); }
has() { echo "$1" | grep -qF "$2"; }

if cmake -S examples -B "$BIN" >/dev/null 2>&1 && cmake --build "$BIN" >/dev/null 2>&1; then
  ok "build examples"
else ng "build examples"; echo "passed=$pass failed=$fail"; exit 1; fi

if cmake -S solutions -B "$SBIN" >/dev/null 2>&1 && cmake --build "$SBIN" >/dev/null 2>&1; then
  ok "build solutions"
else ng "build solutions"; echo "passed=$pass failed=$fail"; exit 1; fi

o=$(timeout 10 "$BIN/ex01_byteorder"); rc=$?
if [ $rc -eq 0 ] && has "$o" "network bytes of 0x12345678: 12 34 56 78" \
   && has "$o" "round-trip: 0x12345678 -> htonl -> ntohl = 0x12345678" \
   && echo "$o" | grep -qE "^host byte order: (little|big)-endian$"; then
  ok "ex01 byte order"
else ng "ex01 (rc=$rc out=[$o])"; fi

o=$(timeout 20 "$BIN/ex02_tcp_echo"); rc=$?
if [ $rc -eq 0 ] && has "$o" "echo OK: hello socket"; then
  ok "ex02 TCP echo"
else ng "ex02 (rc=$rc out=[$o])"; fi

if g++ -std=c++11 -g -no-pie -fsanitize=thread -pthread examples/ex02_tcp_echo.cpp -o "$BIN/tsan_echo" 2>/dev/null; then
  o=$(timeout 120 setarch "$(uname -m)" -R "$BIN/tsan_echo" 2>&1); rc=$?
  if [ $rc -eq 0 ] && has "$o" "echo OK: hello socket" && ! has "$o" "ThreadSanitizer"; then
    ok "ex02 TSan-clean"
  else ng "ex02 tsan (rc=$rc out=[$(echo "$o" | head -5)])"; fi
else ng "tsan build of ex02 failed"; fi

o=$(timeout 20 "$BIN/ex03_send_recv_all"); rc=$?
if [ $rc -eq 0 ] && has "$o" "received 100000 bytes" && has "$o" "checksum match"; then
  ok "ex03 send/recv_all 100KB exact"
else ng "ex03 (rc=$rc out=[$o])"; fi

o=$(timeout 20 "$SBIN/ex02_multi_round_echo"); rc=$?
if [ $rc -eq 0 ] && has "$o" "multi-round echo OK: 3/3"; then
  ok "solution ex02 multi-round echo"
else ng "solution ex02 (rc=$rc out=[$o])"; fi

# UDP 回显在某些受限沙箱里会被禁（本沙箱实测：裸 UDP sendto 即报 EPERM，
# 与代码无关）。验收先探测 UDP 回环是否可用：不可用时明示跳过、不计失败；
# 在自己的开发机上跑通才是该项的正式验收。
if python3 -c 'import socket; s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM); s.sendto(b"p", ("127.0.0.1", 9)); s.close()' 2>/dev/null; then
  o=$(timeout 20 "$SBIN/ex03_udp_echo"); rc=$?
  if [ $rc -eq 0 ] && has "$o" "udp echo OK: hello udp"; then
    ok "solution ex03 UDP echo"
  else ng "solution ex03 (rc=$rc out=[$o])"; fi
else
  echo "SKIP: solution ex03 UDP echo (UDP loopback blocked in this sandbox)"
fi

echo "----"; echo "passed=$pass failed=$fail"
[ $fail -eq 0 ]
