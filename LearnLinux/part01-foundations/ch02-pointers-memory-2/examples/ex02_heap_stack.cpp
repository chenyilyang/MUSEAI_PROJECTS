// ex02_heap_stack.cpp —— 栈与堆：两种内存，两种生命周期
// 编译基线：C++11
#include <cstdio>

int g_value = 7;   // 全局变量：住在数据段，程序全程都在

// 在堆上建数组并返回指针：函数返回后数组依然活着，直到有人 delete[]
int* makeOnHeap(int n) {
    int* a = new int[n];
    for (int i = 0; i < n; ++i) {
        a[i] = 100 + i;
    }
    return a;
}

void showRegions() {
    static int s_value = 8;   // 静态局部变量：也住在数据段，只初始化一次
    int local = 9;            // 局部变量：住在栈上，函数返回就消失
    int* heap = new int(10);  // 堆变量：手动申请，手动释放

    // 四个地址分属不同区域（具体数值每次运行都会变，看"不在同一片"即可）
    std::printf("global  (data seg): %p\n", (void*)&g_value);
    std::printf("static  (data seg): %p\n", (void*)&s_value);
    std::printf("local   (stack)   : %p\n", (void*)&local);
    std::printf("heap var (heap)   : %p\n", (void*)heap);

    delete heap;
}

int main() {
    showRegions();

    int* arr = makeOnHeap(3);         // 堆数组跨函数存活
    std::printf("heap array: %d %d %d\n", arr[0], arr[1], arr[2]);  // 100 101 102

    delete[] arr;                     // 数组用 delete[]，单个对象用 delete，两者必须配对
    arr = nullptr;
    return 0;
}
