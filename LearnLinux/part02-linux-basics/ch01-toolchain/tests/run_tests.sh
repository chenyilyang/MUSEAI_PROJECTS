#!/usr/bin/env bash
# 第二部分第 1 章（工具链与环境搭建）验收测试 —— 2026-10-23 补齐。
# 本章示例是命令序列而非单一工程，脚本在临时目录里重演四组实验并断言结果。
# 前置：gdb 已安装（本章环境要求）。地址/行数类数值只断言关系与量级。
set -u
cd "$(dirname "$0")/.."
SRC="$PWD/examples"
pass=0; fail=0
ok() { echo "PASS: $1"; pass=$((pass+1)); }
ng() { echo "FAIL: $1"; fail=$((fail+1)); }
has() { echo "$1" | grep -qF "$2"; }

T=$(mktemp -d)

# ---- ex01：四阶段产物 + nm 符号 + 运行输出 ----
mkdir "$T/ex01" && cp "$SRC/ex01_build_stages/hello.cpp" "$T/ex01/"
(
  cd "$T/ex01" || exit 1
  g++ -E hello.cpp -o hello.ii && g++ -S hello.cpp -o hello.s \
    && g++ -c hello.cpp -o hello.o && g++ hello.o -o hello
) >/dev/null 2>&1
o=$(cd "$T/ex01" && ./hello)
nmo=$(cd "$T/ex01" && nm hello.o)
lines=$(wc -l < "$T/ex01/hello.ii")
if [ -f "$T/ex01/hello.ii" ] && [ -f "$T/ex01/hello.s" ] && [ "$lines" -gt 500 ] \
   && echo "$nmo" | grep -qE " T main$" && echo "$nmo" | grep -qE " U puts$" \
   && [ "$o" = "hello, toolchain" ]; then
  ok "ex01 four stages, nm T main / U puts, runs"
else ng "ex01 (lines=$lines nm=[$nmo] out=[$o])"; fi

# ---- ex02：静态库 vs 动态库 ----
mkdir "$T/ex02" && cp "$SRC/ex02_libraries/"* "$T/ex02/"
(
  cd "$T/ex02" || exit 1
  g++ -c calc.cpp -o calc.o
  ar rcs libcalc.a calc.o
  g++ main.cpp libcalc.a -o app_static
  g++ -fPIC -shared calc.cpp -o libcalc.so
  g++ main.cpp -L. -lcalc -o app_shared
) >/dev/null 2>&1
s_out=$(cd "$T/ex02" && ./app_static)
s_ldd=$(cd "$T/ex02" && ldd app_static)
bare_err=$(cd "$T/ex02" && ./app_shared 2>&1); bare_rc=$?
d_out=$(cd "$T/ex02" && LD_LIBRARY_PATH=. ./app_shared)
d_ldd=$(cd "$T/ex02" && LD_LIBRARY_PATH=. ldd app_shared)
if has "$s_out" "add(3, 4) = 7" && has "$s_out" "mul(3, 4) = 12" \
   && ! echo "$s_ldd" | grep -qi "calc" \
   && [ $bare_rc -ne 0 ] && has "$bare_err" "cannot open shared object file" \
   && has "$d_out" "add(3, 4) = 7" && echo "$d_ldd" | grep -qE "libcalc\.so => .*libcalc\.so"; then
  ok "ex02 static has no calc in ldd; shared needs LD_LIBRARY_PATH"
else ng "ex02 (bare_rc=$bare_rc bare_err=[$bare_err])"; fi

# ---- ex03：CMake 工程输出与手工构建一致 ----
mkdir "$T/ex03"
cp "$SRC/ex03_cmake_project/"*.cpp "$SRC/ex03_cmake_project/"*.h "$SRC/ex03_cmake_project/CMakeLists.txt" "$T/ex03/"
(
  cd "$T/ex03" || exit 1
  cmake -S . -B build >/dev/null 2>&1 && cmake --build build >/dev/null 2>&1
  g++ calc.cpp greet.cpp main.cpp -o demo_manual
) >/dev/null 2>&1
c_out=$(cd "$T/ex03" && ./build/demo)
m_out=$(cd "$T/ex03" && ./demo_manual)
if [ -n "$c_out" ] && [ "$c_out" = "$m_out" ] && has "$c_out" "add(3, 4) = 7"; then
  ok "ex03 cmake output == manual build output"
else ng "ex03 (cmake=[$c_out] manual=[$m_out])"; fi

# ---- ex04：buggy=56 / fixed=55 + GDB 批处理三查 ----
mkdir "$T/ex04" && cp "$SRC/ex04_gdb/"*.cpp "$T/ex04/"
(
  cd "$T/ex04" || exit 1
  g++ -g -O0 buggy_sum.cpp -o buggy_sum
  g++ -g -O0 fixed_sum.cpp -o fixed_sum
) >/dev/null 2>&1
b_out=$(cd "$T/ex04" && ./buggy_sum)
f_out=$(cd "$T/ex04" && ./fixed_sum)
if has "$b_out" "sum 1..10 = 56" && has "$f_out" "sum 1..10 = 55"; then
  ok "ex04 buggy 56 / fixed 55"
else ng "ex04 outputs (buggy=[$b_out] fixed=[$f_out])"; fi

if command -v gdb >/dev/null 2>&1; then
  g1=$(cd "$T/ex04" && gdb -batch -ex 'break buggy_sum.cpp:10' -ex run -ex 'print sum' ./buggy_sum 2>/dev/null)
  g2=$(cd "$T/ex04" && gdb -batch -ex 'break buggy_sum.cpp:8' -ex 'ignore 1 9' -ex run -ex 'print i' -ex 'print sum' ./buggy_sum 2>/dev/null)
  g3=$(cd "$T/ex04" && gdb -batch -ex 'break buggy_sum.cpp:8' -ex run -ex 'print sum' ./buggy_sum 2>/dev/null)
  if echo "$g1" | grep -q '\$1 = 56' && echo "$g2" | grep -q '= 10' && echo "$g2" | grep -q '= 46' \
     && echo "$g3" | grep -q '\$1 = 1'; then
    ok "ex04 gdb session: sum=56 at printf; 10th hit i=10 sum=46; 1st hit sum=1"
  else ng "ex04 gdb (g1=[$g1] g2=[$g2] g3=[$g3])"; fi
else
  ng "gdb not installed (chapter prerequisite)"
fi

rm -rf "$T"
echo "----"; echo "passed=$pass failed=$fail"
[ $fail -eq 0 ]
