# bypSerial - 串口监视器

bypSerial 是 BYP 工具系列的串口通信调试工具，用于嵌入式开发中的串口数据监视和发送。

## 功能特性

- ✅ 自动枚举可用串口
- ✅ 支持常用波特率（9600 ~ 921600）
- ✅ 实时接收数据显示（文本/HEX）
- ✅ 数据发送（文本/HEX）
- ✅ 收发字节统计
- ✅ 日志保存功能
- ✅ 自动滚动开关

## 编译步骤

### 1. 安装 Qt6

确保安装了 Qt6，并包含 **QSerialPort** 模块：

```bash
# Windows (MSVC)
# 从 https://www.qt.io/download 下载 Qt Online Installer
# 安装时勾选 "Qt 6.x" 和 "Qt Serial Port"

# Linux (Ubuntu/Debian)
sudo apt install qt6-base-dev qt6-serialport-dev

# macOS
brew install qt
```

### 2. 打开项目

```bash
# 方法 1: Qt Creator
# 打开 bypSerial.pro 文件

# 方法 2: 命令行（先确保 Qt 的 bin 与 MinGW 的 bin 已加入 PATH）
qmake bypSerial.pro
mingw32-make -j4    # 或 nmake (MSVC)
```

### 3. 运行

```bash
# Windows（MinGW 构建，产物在 build-release\release\）
build-release\release\bypSerial.exe

# Linux
./bypSerial

# macOS
./bypSerial.app/Contents/MacOS/bypSerial
```

### 4. 一键出包（Windows）

右键 `build-release.ps1` → **使用 PowerShell 运行**。脚本依次执行 qmake → 编译 →
`windeployqt` 打包 Qt 依赖，产物在 `build-release\release\`；整个 `release` 目录
可直接拷到没有安装 Qt 的机器上运行。

> 前提：`qmake`、`mingw32-make`、`windeployqt` 已在 PATH 中（Qt 的 `bin` 目录与
> Qt Tools 下 MinGW 的 `bin` 目录）。

## 使用说明

### 连接串口

1. 点击 **🔄 刷新** 按钮，扫描可用串口
2. 从下拉列表选择串口（如 COM3）
3. 选择波特率（默认 115200）
4. 点击 **打开串口**

### 接收数据

- 接收到的数据会实时显示在日志区域
- 勾选 **HEX 显示** 可以十六进制查看
- 勾选 **自动滚动** 保持最新数据可见

### 发送数据

1. 在底部输入框输入要发送的内容
2. 勾选 **HEX 发送** 可以发送十六进制数据
3. 点击 **发送** 或按回车键

### 保存日志

点击 **保存日志** 按钮，将当前会话的所有收发数据保存为文本文件。

## 项目结构

```
bypSerial/
├── bypSerial.pro       # Qt 项目文件
├── main.cpp            # 程序入口
├── mainwindow.h        # 主窗口头文件
├── mainwindow.cpp      # 主窗口实现
├── mainwindow.ui       # 界面定义（当前仅有空壳，控件在 mainwindow.cpp 中构建）
├── bypSerial.qrc       # 资源文件（程序内图标）
├── bypSerial.png       # 应用图标（由 qrc 内嵌）
├── bypSerial.ico       # EXE 图标（由 RC_ICONS 内嵌）
├── build-release.ps1   # Windows 一键 Release 构建 + windeployqt
├── .gitignore
├── LICENSE
├── CHANGELOG.md        # 版本变更记录
└── README.md
```

## 依赖

- Qt 6.x（`core`、`gui`、`widgets`、`serialport`）

已验证平台：Windows 11 + Qt 6.12.0 + MinGW 13.1（GCC 64-bit）。
Linux / macOS 理论上可以编译，但未做实际验证。

## 已知限制

- 固定 8N1，暂不提供数据位 / 校验位 / 停止位选择
- 无 DTR/RTS 控制，暂不支持依赖该信号复位的开发板（如 ESP32 自动下载）
- 暂无发送行尾追加（`\r\n`）选项，需要时可用 HEX 发送 `0D 0A`
- 文本模式按 `\n` 分帧：对端长时间不发换行时，缓冲区超过 4 KB 会强制输出一行

## 版本与发布

版本号只在 `bypSerial.pro` 的 `VERSION` 一处维护。窗口标题、帮助/关于对话框、
exe 的文件版本信息、`build-release.ps1` 打出的发布包名全部由它派生，不会再出现
代码里写死版本号、忘记同步的情况。

递增判据：

- **主版本**（`1.x` → `2.0`）：新增功能能力，如定时发送、多串口会话
- **次版本**（`1.0` → `1.1`）：界面重做、既有缺陷全修、框架/工具链迁移等精修升级
- **修订号**（`1.0.0` → `1.0.1`）：少量单点修复，功能集合不变

发版流程：

1. 改 `bypSerial.pro` 的 `VERSION`
2. 在 [CHANGELOG.md](CHANGELOG.md) 的「未发布」下补条目，发版时改成版本号 + 日期
3. 运行 `build-release.ps1`，得到 `build-release\release\` 与 `bypSerial-v<版本>-win64.zip`
4. `git commit` → `git tag -a v<版本> -m "bypSerial v<版本>"` → `git push --tags`
5. GitHub Releases 上传 zip，正文用 CHANGELOG 对应小节

## 许可证

[MIT License](LICENSE) © 2026 白毅平

## 作者

BYP Tools Series · [@eebyp](https://github.com/eebyp)

---

**版本**: 1.0.0（以 `bypSerial.pro` 的 `VERSION` 为准）  
**更新日期**: 2026-11-08
