# Windows 11 全局搜索工具

一个轻量级的 Windows 桌面搜索工具，通过全局快捷键快速调用搜索功能。

## 功能特点

- 🚀 **轻量高效**：可执行文件仅 246KB，内存占用极低
- ⌨️ **全局快捷键**：支持多种快捷键组合，自动检测可用快捷键
- 🔍 **快速搜索**：输入内容后按回车直接打开默认浏览器搜索
- 🌏 **中文支持**：完美支持中文搜索，正确的UTF-8 URL编码
- 🎨 **现代UI**：深色主题，圆角边框，居中显示，32号大字体适配搜索框
- 🔒 **防多开**：智能检测，同时只能运行一个实例
- 🤫 **静默运行**：启动后完全静默，无任何提示窗口
- ⚡ **快速响应**：原生 C++ 开发，性能优异
- 🎯 **输入法优化**：完美支持微软拼音输入法，自动显示中文候选词

## 快速开始

### 下载使用

1. 下载 `search_tool_silent.exe`
2. 双击运行（程序会自动后台运行）
3. 使用快捷键呼出搜索框：
   - 优先使用 `Alt + Space`
   - 如果被占用，自动尝试 `Alt + Q`、`Ctrl + Space` 或 `F8`
4. 输入搜索内容，按 `Enter` 搜索
5. 按 `Esc` 或点击窗口外部隐藏搜索框

### 系统要求

- Windows 10/11
- 无需额外依赖（静态链接编译）

## 编译说明

### 环境准备

#### 方法一：使用 MinGW-w64（推荐）

1. 下载并安装 [MinGW-w64](https://www.mingw-w64.org/)
2. 确保 `g++.exe` 在系统 PATH 中
3. 打开命令提示符或 PowerShell

#### 方法二：使用 Visual Studio

1. 安装 Visual Studio 2019/2022
2. 安装 C++ 桌面开发工作负载
3. 打开 "Developer Command Prompt for VS"

### 编译命令

```bash
# 使用 MinGW-w64 编译
g++ -O2 -mwindows -static-libgcc -static-libstdc++ -static -s -DNDEBUG -DUNICODE -D_UNICODE search_tool.cpp -o search_tool_silent.exe -lgdi32 -luser32 -lshell32 -ldwmapi -lcomctl32 -limm32

# 使用 Visual Studio 编译
cl /O2 /MT /GL /DNDEBUG /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" search_tool.cpp /Fe:search_tool_silent.exe /link /LTCG user32.lib gdi32.lib shell32.lib dwmapi.lib comctl32.lib imm32.lib
```

### 编译参数说明

- `-O2`：最高级别优化
- `-mwindows`：Windows GUI 应用程序
- `-static-*`：静态链接，无需运行时依赖
- `-s`：剥离调试信息，减小文件大小
- `-DUNICODE -D_UNICODE`：启用 Unicode 支持

## 项目结构

```
├── search_tool.cpp          # 主程序源代码
├── resource.h               # 资源头文件
├── search_tool_silent.exe   # 编译后的可执行文件
└── README.md               # 项目说明文档
```

## 技术实现

### 核心技术

- **编程语言**：C++
- **GUI框架**：原生 Windows API
- **编译器**：MinGW-w64/GCC 或 MSVC
- **链接方式**：静态链接

### 主要功能实现

1. **全局快捷键监听**：使用 `RegisterHotKey` API
2. **防多开机制**：使用全局命名互斥锁
3. **现代UI效果**：DWM 毛玻璃效果 + 圆角窗口
4. **搜索功能**：ShellExecute 调用默认浏览器
5. **中文URL编码**：正确的UTF-8编码支持中文字符搜索
6. **字体优化**：使用32号微软雅黑字体，大字体显示
7. **输入法优化**：完美支持微软拼音输入法，自动显示候选词窗口
8. **焦点管理**：智能焦点控制，避免第一次呼出自动退出

### 快捷键优先级

程序会按以下顺序尝试注册快捷键：
1. `Alt + Space`（默认）
2. `Alt + Q`
3. `Ctrl + Space`
4. `F8`（最后备选）

## 性能优化

- **文件大小优化**：静态链接 + 调试信息剥离
- **内存占用优化**：最小化全局变量，及时释放资源
- **启动速度优化**：减少初始化操作，延迟加载
- **响应速度优化**：使用高效的 Windows API

## 常见问题

### Q: 快捷键不生效？
A: 程序会自动检测可用的快捷键，如果 Alt+Space 被占用，会自动尝试其他组合。

### Q: 程序无法启动？
A: 请确保系统已安装 Windows 10/11，并检查防病毒软件是否误报。

### Q: 如何完全退出程序？
A: 在任务管理器中结束 `search_tool_silent.exe` 进程。

## 许可证

本项目采用 MIT 许可证，详见 LICENSE 文件。

## 贡献

欢迎提交 Issue 和 Pull Request！

---

**注意**：首次运行时，某些防病毒软件可能会误报，请添加到信任列表。