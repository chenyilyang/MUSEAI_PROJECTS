// ex01_raii_file.cpp —— RAII：把资源绑在栈对象的生命周期上
// 无论函数从哪条路径出去（正常走完、中途 return），离开作用域时
// 析构函数都会被调用——释放动作因此不可能被"忘掉"。
// 编译基线：C++11
#include <cstdio>

class FileWriter {
public:
    explicit FileWriter(const char* path) : f_(std::fopen(path, "w")) {
        std::printf("opened: %s\n", path);
    }
    ~FileWriter() {                          // 析构即关闭，不管怎么离开
        if (f_ != NULL) std::fclose(f_);
        std::printf("closed automatically\n");
    }
    void writeLine(const char* text) {
        if (f_ != NULL) std::fputs(text, f_);
    }
private:
    std::FILE* f_;
    FileWriter(const FileWriter&);             // 禁止拷贝：一个文件只能有一个主人
    FileWriter& operator=(const FileWriter&);
};

void writeNormal(const char* path) {
    FileWriter fw(path);
    fw.writeLine("hello raii\n");
    // 正常走到结尾：出作用域时自动关闭
}

void writeEarlyReturn(const char* path) {
    FileWriter fw(path);
    fw.writeLine("early return\n");
    return;    // 提前返回：析构照样执行——裸指针写法最容易在这里漏 fclose
}

void printFile(const char* path) {
    std::FILE* f = std::fopen(path, "r");
    char buf[64] = {0};
    if (f != NULL) {
        std::fgets(buf, sizeof(buf), f);
        std::fclose(f);
    }
    std::printf("content of %s: %s", path, buf);
}

int main() {
    writeNormal("ex01_normal.txt");
    writeEarlyReturn("ex01_early.txt");
    printFile("ex01_normal.txt");     // hello raii
    printFile("ex01_early.txt");      // early return —— 两份文件都完整落盘
    return 0;
}
