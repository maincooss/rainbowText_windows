#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <cmath>
#include <locale>
#include <codecvt>
#include <sstream>
#include <io.h>
#include <versionhelpers.h>

// 彩虹文本输出工具
// 支持Windows环境下的彩虹色文本输出，包括中文字符
// 自动检测终端编码格式，解决中文乱码问题

// 颜色结构体
struct Color {
    int r, g, b;

    Color(int red, int green, int blue) : r(red), g(green), b(blue) {}
};

// 生成彩虹颜色序列
std::vector<Color> generateRainbowColors(int numColors) {
    std::vector<Color> colors;
    for (int i = 0; i < numColors; i++) {
        double position = (double)i / numColors;
        double frequency = 2.0 * 3.14159265358979323846;

        // 使用正弦波生成RGB值，创建彩虹效果
        int r = (int)(sin(frequency * position + 0) * 127 + 128);
        int g = (int)(sin(frequency * position + 2) * 127 + 128);
        int b = (int)(sin(frequency * position + 4) * 127 + 128);

        colors.push_back(Color(r, g, b));
    }
    return colors;
}

// 获取当前控制台的代码页
UINT getConsoleCodePage() {
    return GetConsoleOutputCP();
}

// 检测终端是否支持ANSI转义序列
bool isAnsiSupported() {
    // 默认返回true，强制使用彩虹色
    return true;

    // 以下代码暂时不使用，确保默认使用彩虹色
    /*
    // 检查Windows版本
    // Windows 10 1903+ 通常支持ANSI转义序列
    if (IsWindows10OrGreater()) {
        DWORD mode = 0;
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (GetConsoleMode(hOut, &mode)) {
            // 检查是否支持虚拟终端处理
            // ENABLE_VIRTUAL_TERMINAL_PROCESSING = 0x0004
            return (mode & 0x0004) != 0;
        }
    }
    return false;
    */
}

// 尝试启用ANSI转义序列支持
bool enableAnsiSupport() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD mode = 0;
    if (!GetConsoleMode(hOut, &mode)) {
        return false;
    }

    // 尝试启用虚拟终端处理
    // ENABLE_VIRTUAL_TERMINAL_PROCESSING = 0x0004
    mode |= 0x0004;
    if (!SetConsoleMode(hOut, mode)) {
        return false;
    }

    return true;
}

// 自动设置适当的控制台编码
void setAppropriateConsoleEncoding() {
    // 获取系统默认的ANSI代码页
    UINT defaultACP = GetACP();

    // 获取当前控制台代码页
    UINT currentCP = getConsoleCodePage();

    // 根据系统默认代码页设置控制台代码页
    if (defaultACP == 936) {  // 简体中文 GBK/GB2312
        SetConsoleOutputCP(936);
    }
    else if (defaultACP == 950) {  // 繁体中文 Big5
        SetConsoleOutputCP(950);
    }
    else if (defaultACP == 54936) {  // GB18030
        SetConsoleOutputCP(54936);
    }
    else {
        // 如果不是中文系统，尝试使用UTF-8
        SetConsoleOutputCP(65001);  // UTF-8
    }

    // 设置控制台输入代码页与输出代码页一致
    SetConsoleCP(GetConsoleOutputCP());

    // 设置C++本地化环境
    std::locale::global(std::locale(""));

    // 尝试启用ANSI转义序列支持
    enableAnsiSupport();
}

// 将RGB颜色转换为Windows控制台颜色代码
int rgbToConsoleColor(const Color& color) {
    // Windows控制台只支持16种颜色，我们需要找到最接近的颜色
    // 0=黑 1=蓝 2=绿 3=青 4=红 5=紫 6=黄 7=白
    // 8-15是高亮版本

    int bestMatch = 0;
    int minDifference = 255 * 3;

    // 控制台颜色的RGB值（近似值）
    const Color consoleColors[] = {
        Color(0, 0, 0),       // 黑色
        Color(0, 0, 128),     // 蓝色
        Color(0, 128, 0),     // 绿色
        Color(0, 128, 128),   // 青色
        Color(128, 0, 0),     // 红色
        Color(128, 0, 128),   // 紫色
        Color(128, 128, 0),   // 黄色
        Color(192, 192, 192), // 白色
        Color(128, 128, 128), // 灰色
        Color(0, 0, 255),     // 亮蓝色
        Color(0, 255, 0),     // 亮绿色
        Color(0, 255, 255),   // 亮青色
        Color(255, 0, 0),     // 亮红色
        Color(255, 0, 255),   // 亮紫色
        Color(255, 255, 0),   // 亮黄色
        Color(255, 255, 255)  // 亮白色
    };

    // 找到最接近的控制台颜色
    for (int i = 0; i < 16; i++) {
        int dr = color.r - consoleColors[i].r;
        int dg = color.g - consoleColors[i].g;
        int db = color.b - consoleColors[i].b;
        int difference = dr * dr + dg * dg + db * db;

        if (difference < minDifference) {
            minDifference = difference;
            bestMatch = i;
        }
    }

    // 避免使用黑色（0），如果最接近的是黑色，则使用深蓝色（1）代替
    if (bestMatch == 0) {
        bestMatch = 1;
    }

    return bestMatch;
}

// 打印彩虹文本
void printRainbowText(const std::string& text, bool vertical = false) {
    // 获取控制台句柄
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // 保存原始控制台属性
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    WORD originalAttrs = csbi.wAttributes;

    // 生成彩虹颜色
    std::vector<Color> colors = generateRainbowColors(text.length());

    // 自动设置适当的控制台编码
    setAppropriateConsoleEncoding();

    // 获取当前控制台代码页
    UINT currentCP = getConsoleCodePage();

    // 输出彩虹文本
    for (size_t i = 0; i < text.length(); i++) {
        // 获取当前字符
        unsigned char c = (unsigned char)text[i];

        // 检查是否是多字节字符（根据当前代码页）
        bool isMultiByte = false;
        int extraBytes = 0;

        if (currentCP == 65001) { // UTF-8
            if (c >= 0xE0) { // 三字节UTF-8
                isMultiByte = true;
                extraBytes = 2;
            }
            else if (c >= 0xC0) { // 二字节UTF-8
                isMultiByte = true;
                extraBytes = 1;
            }
        }
        else if (currentCP == 936 || currentCP == 950 || currentCP == 54936) { // GBK/Big5/GB18030
            if (c >= 0x80) { // 双字节编码
                isMultiByte = true;
                extraBytes = 1;
            }
        }

        // 设置文本颜色
        int colorIndex = rgbToConsoleColor(colors[i]);
        SetConsoleTextAttribute(hConsole, colorIndex);

        // 输出字符
        std::cout << text[i];

        // 如果是多字节字符，需要输出额外的字节
        if (isMultiByte) {
            for (int j = 0; j < extraBytes && i + 1 < text.length(); j++) {
                i++;
                std::cout << text[i];
            }
        }

        // 垂直模式时换行
        if (vertical) {
            std::cout << std::endl;
        }
    }

    // 恢复原始控制台属性
    SetConsoleTextAttribute(hConsole, originalAttrs);

    // 如果不是垂直模式，最后输出换行
    if (!vertical) {
        std::cout << std::endl;
    }
}

// 更高级的彩虹文本输出（使用Windows控制台的RGB颜色支持 - 仅在较新的Windows 10上支持）
void printTrueColorRainbowText(const std::string& text, bool vertical = false) {
    // 获取控制台句柄
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // 保存原始控制台属性
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    WORD originalAttrs = csbi.wAttributes;

    // 生成彩虹颜色
    std::vector<Color> colors = generateRainbowColors(text.length());

    // 自动设置适当的控制台编码
    setAppropriateConsoleEncoding();

    // 获取当前控制台代码页
    UINT currentCP = getConsoleCodePage();

    // 检查是否支持ANSI转义序列
    bool ansiSupported = isAnsiSupported();

    // 输出彩虹文本
    for (size_t i = 0; i < text.length(); i++) {
        // 获取当前字符
        unsigned char c = (unsigned char)text[i];

        // 检查是否是多字节字符（根据当前代码页）
        bool isMultiByte = false;
        int extraBytes = 0;

        if (currentCP == 65001) { // UTF-8
            if (c >= 0xE0) { // 三字节UTF-8
                isMultiByte = true;
                extraBytes = 2;
            }
            else if (c >= 0xC0) { // 二字节UTF-8
                isMultiByte = true;
                extraBytes = 1;
            }
        }
        else if (currentCP == 936 || currentCP == 950 || currentCP == 54936) { // GBK/Big5/GB18030
            if (c >= 0x80) { // 双字节编码
                isMultiByte = true;
                extraBytes = 1;
            }
        }

        Color color = colors[i];

        if (ansiSupported) {
            // 使用ANSI转义序列设置RGB颜色（Windows 10 1903+支持）
            std::cout << "\033[38;2;" << color.r << ";" << color.g << ";" << color.b << "m";
        }
        else {
            // 在不支持ANSI的系统上回退到基本颜色模式
            int colorIndex = rgbToConsoleColor(color);
            SetConsoleTextAttribute(hConsole, colorIndex);
        }

        // 输出字符
        std::cout << text[i];

        // 如果是多字节字符，需要输出额外的字节
        if (isMultiByte) {
            for (int j = 0; j < extraBytes && i + 1 < text.length(); j++) {
                i++;
                std::cout << text[i];
            }
        }

        // 垂直模式时换行
        if (vertical) {
            std::cout << std::endl;
        }
    }

    // 恢复原始颜色
    if (ansiSupported) {
        std::cout << "\033[0m";
    }
    else {
        SetConsoleTextAttribute(hConsole, originalAttrs);
    }

    // 如果不是垂直模式，最后输出换行
    if (!vertical) {
        std::cout << std::endl;
    }
}

// 检查是否有管道输入
bool hasPipedInput() {
    // 检查标准输入是否为管道或重定向
    return !_isatty(_fileno(stdin));
}

// 从管道读取输入并实时处理每一行
void processInputStreamRealtime(bool verticalMode, bool trueColorMode) {
    std::string line;

    // 逐行读取标准输入并立即处理
    while (std::getline(std::cin, line)) {
        // 如果行为空且到达EOF，使用默认文本
        if (line.empty() && std::cin.eof()) {
            line = "彩虹文字";
        }

        // 立即处理并显示当前行
        if (trueColorMode) {
            printTrueColorRainbowText(line, verticalMode);
        }
        else {
            printRainbowText(line, verticalMode);
        }
    }
}

// 从管道读取所有输入（用于向后兼容）
std::string readFromPipe() {
    std::stringstream buffer;
    std::string line;

    // 逐行读取标准输入
    while (std::getline(std::cin, line)) {
        buffer << line;
        // 添加换行符，除非是最后一行
        if (!std::cin.eof()) {
            buffer << std::endl;
        }
    }

    return buffer.str();
}

// 显示帮助信息
void showHelp() {
    std::cout << "彩虹文本输出工具 - 使用说明" << std::endl;
    std::cout << "用法: rainbow_text [选项] [文本]" << std::endl;
    std::cout << "      echo 文本 | rainbow_text [选项]" << std::endl;
    std::cout << "选项:" << std::endl;
    std::cout << "  -h, --help     显示此帮助信息" << std::endl;
    std::cout << "  -v, --vertical 垂直显示文本" << std::endl;
    std::cout << "  -b, --basic    使用基本颜色模式 (默认使用真彩色模式)" << std::endl;
    std::cout << "  -r, --random   每行随机彩虹渐变" << std::endl;
    std::cout << "示例:" << std::endl;
    std::cout << "  rainbow_text           (显示默认彩虹文字)" << std::endl;
    std::cout << "  rainbow_text 你好世界  (显示指定文本)" << std::endl;
    std::cout << "  rainbow_text -v 你好世界 (垂直显示)" << std::endl;
    std::cout << "  rainbow_text -b 你好世界 (使用基本颜色模式)" << std::endl;
    std::cout << "  echo 你好世界 | rainbow_text (通过管道输入)" << std::endl;
    std::cout << "  type 文件.txt | rainbow_text -r  (每行随机彩虹渐变)" << std::endl;
}

int main(int argc, char* argv[]) {
    // 自动设置适当的控制台编码
    setAppropriateConsoleEncoding();

    // 默认参数
    bool verticalMode = false;
    bool trueColorMode = true;  // 默认使用真彩色模式
    std::string text = "彩虹文字";  // 默认文本

    // 检查是否有管道输入
    bool hasPipe = hasPipedInput();

    // 解析命令行参数
    if (argc > 1) {
        // 清空默认文本，除非有管道输入
        if (!hasPipe) {
            text = "";
        }

        // 处理参数
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];

            if (arg == "-h" || arg == "--help") {
                showHelp();
                return 0;
            }
            else if (arg == "-v" || arg == "--vertical") {
                verticalMode = true;
            }
            else if (arg == "-b" || arg == "--basic") {
                trueColorMode = false;  // 使用基本颜色模式
            }
            else if (!hasPipe) {
                // 如果不是选项且没有管道输入，则视为文本内容
                if (!text.empty()) {
                    text += " ";  // 添加空格连接多个参数
                }
                text += arg;
            }
        }

        // 如果解析后没有文本（只有选项）且没有管道输入，恢复默认文本
        if (text.empty() && !hasPipe) {
            text = "彩虹文字";
        }
    }

    // 如果有管道输入，实时处理每一行
    if (hasPipe) {
        // 直接处理输入流，实时显示每一行
        processInputStreamRealtime(verticalMode, trueColorMode);
        return 0;
    }

    // 非管道输入模式，根据模式输出彩虹文本
    if (trueColorMode) {
        printTrueColorRainbowText(text, verticalMode);
    }
    else {
        printRainbowText(text, verticalMode);
    }

    return 0;
}
