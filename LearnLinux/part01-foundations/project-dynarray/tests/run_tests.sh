#!/usr/bin/env bash
# 阶段项目 DynArray 统一验收入口 —— 2026-10-23 补齐。
# 本项目当初就是验收测试先行（固定验收程序 test_dynarray.cpp 先于参考实现），
# 此脚本只是把当年的三重验证固化成一键复跑：
#   1) 参考实现普通构建：须打印 ALL TESTS PASSED、退出码 0；
#   2) 参考实现 ASan+UBSan 构建：须同样通过且无任何 sanitizer 报告；
#   3) 骨架构建：可编译，但必须挂在验收上（退出码非 0）——证明验收有区分力。
set -u
cd "$(dirname "$0")/.."
pass=0; fail=0
ok() { echo "PASS: $1"; pass=$((pass+1)); }
ng() { echo "FAIL: $1"; fail=$((fail+1)); }

# 1) solutions 正常构建
if cmake -S solutions -B solutions/build >/dev/null 2>&1 && cmake --build solutions/build >/dev/null 2>&1; then
  out=$(./solutions/build/test_dynarray 2>&1); rc=$?
  if [ $rc -eq 0 ] && echo "$out" | grep -q "ALL TESTS PASSED"; then
    ok "solutions passes acceptance"
  else ng "solutions run (rc=$rc out=[$out])"; fi
else ng "solutions build"; fi

# 2) solutions ASan+UBSan 构建
if cmake -S solutions -B solutions/build-asan -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -g" >/dev/null 2>&1 \
   && cmake --build solutions/build-asan >/dev/null 2>&1; then
  out=$(./solutions/build-asan/test_dynarray 2>&1); rc=$?
  if [ $rc -eq 0 ] && echo "$out" | grep -q "ALL TESTS PASSED" && ! echo "$out" | grep -qi "sanitizer"; then
    ok "solutions ASan/UBSan clean"
  else ng "solutions ASan run (rc=$rc)"; fi
else ng "solutions ASan build"; fi

# 3) starter 必须不过
if cmake -S starter -B starter/build >/dev/null 2>&1 && cmake --build starter/build >/dev/null 2>&1; then
  ./starter/build/test_dynarray >/dev/null 2>&1; rc=$?
  if [ $rc -ne 0 ]; then ok "starter fails acceptance as designed (rc=$rc)"; else ng "starter unexpectedly passes"; fi
else ng "starter build (skeleton must compile)"; fi

echo "----"; echo "passed=$pass failed=$fail"
[ $fail -eq 0 ]
