# rainbowText_windows
Output rainbow-colored text on the terminal

用法: rainbow_text [选项] [文本]

选项:
-h, --help     显示此帮助信息
-v, --vertical 垂直显示文本
-b, --basic    使用基本颜色模式 (默认使用真彩色模式)
-r, --random   每行随机彩虹渐变

 示例：
rainbow_text 你好世界  (显示指定文本)
rainbow_text -v 你好世界 (垂直显示)
rainbow_text -b 你好世界 (使用基本颜色模式)
echo 你好世界 | rainbow_text (通过管道输入)
type 文件.txt | rainbow_text -r  (每行随机彩虹渐变)
