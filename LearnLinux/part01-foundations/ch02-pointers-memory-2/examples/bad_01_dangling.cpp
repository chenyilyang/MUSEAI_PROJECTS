// bad_01_dangling.cpp —— 悬垂指针：释放之后继续使用
// 千万不要模仿这种写法。本文件单独编译（见 CMakeLists 的说明），
// 目的是让你亲眼看到"未定义行为"长什么样、检测工具如何抓它。
#include <cstdio>

int main() {
int* p = new int(42);
delete p; // 内存已归还给系统，p 变成悬垂指针

std::printf("*p = %d\n", *p); // 解引用悬垂指针：未定义行为！
// 普通编译运行时，它可能照样打印 42，
// 看起来"没事"——这正是它最危险的地方。
return 0;
}
