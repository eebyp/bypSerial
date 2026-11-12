<p align="center">
  <img src="https://raw.githubusercontent.com/eebyp/bypSerial/main/bypSerial.png" width="96" alt="bypSerial">
</p>

<h1 align="center">bypSerial</h1>

<p align="center">
  <b>轻量、零配置的串口监视器</b><br>
  面向嵌入式调试的 Windows 桌面工具 · Qt 6 + QSerialPort
</p>

<p align="center">
  <a href="https://github.com/eebyp/bypSerial/releases/latest"><img src="https://img.shields.io/github/v/release/eebyp/bypSerial?color=2b9348&label=release" alt="release"></a>
  <a href="LICENSE"><img src="https://img.shields.io/badge/license-MIT-blue.svg" alt="license"></a>
  <img src="https://img.shields.io/badge/Qt-6.12-41cd52.svg" alt="Qt 6.12">
  <img src="https://img.shields.io/badge/platform-Windows%2010%2F11-0078d4.svg" alt="Windows 10/11">
  <a href="https://github.com/eebyp/bypSerial/stargazers"><img src="https://img.shields.io/github/stars/eebyp/bypSerial?style=flat" alt="stars"></a>
</p>

---

## 为什么又写一个串口助手

日常调 STM32 和 CH340 小板子，市面上的串口工具要么功能堆到用不上，要么得先装一堆运行库，要么弹窗催付费。于是按三条原则写了个自己天天在用的：

1. **双击即用** —— 发布包自带 Qt 运行时，解压就能跑，不写注册表、不装驱动
2. **不做二选一** —— 文本和 HEX 是同一份数据的两种看法，切换不改变数据本身
3. **日志一定能落盘** —— 现场没保存就丢了，所以导出 TXT 是一等公民

明确不做的事：多会话、脚本化、跨平台安装包（目前只验证过 Windows）。

---

## 功能

| 环节 | 能力 |
| --- | --- |
| 连接 | 自动枚举串口（显示设备描述）、9600 ~ 921600 常用波特率、波特率可手输 |
| 接收 | 实时显示、文本 / HEX 即时切换、自动滚动可关、RX 字节统计 |
| 发送 | 文本 / HEX 双模式、回车发送、TX 字节统计 |
| 日志 | 按行着色（RX 蓝 / TX 紫 / 系统绿 / 错误红）、一键清空、导出 TXT |
| 健壮性 | 设备拔出自动回滚为未连接并提示、超长无换行数据 4 KB 强制输出 |

---

## 快速开始

### 方式一：直接下载（推荐）

1. 到 [Releases](https://github.com/eebyp/bypSerial/releases/latest) 下载 `bypSerial-v*-win64.zip`
2. 解压到任意目录，双击 `bypSerial.exe`
3. 无需安装 Qt 或其他运行库

### 方式二：从源码构建

**1. 安装 Qt 6**（需勾选 **Qt Serial Port** 模块）

```powershell
# Windows：从 https://www.qt.io/download 安装 Qt Online Installer
# Linux (Ubuntu/Debian)
sudo apt install qt6-base-dev qt6-serialport-dev
# macOS
brew install qt
```

**2. 构建**

```powershell
# 命令行（先把 Qt 的 bin 与 MinGW 的 bin 加入 PATH）
qmake bypSerial.pro
mingw32-make -j4          # 或 nmake (MSVC)

# 也可以用 Qt Creator 直接打开 bypSerial.pro
```

**3. 运行**

```powershell
build-release\release\bypSerial.exe    # MinGW 构建产物落在这里
```

**4. 一键出包（Windows）**

右键 `build-release.ps1` → **使用 PowerShell 运行**。脚本依次执行 qmake → 编译 →
`windeployqt` 打包 Qt 依赖 → 生成 `bypSerial-v<版本>-win64.zip`，整个 `release`
目录可直接拷到没装 Qt 的机器上运行。

> 前提：`qmake`、`mingw32-make`、`windeployqt` 已在 PATH 中（Qt 的 `bin` 与
> Qt Tools 下 MinGW 的 `bin`）。

---

## 界面预览

窗口纵向分三区，从上到下是「串口配置 → 数据日志 → 发送区」，底部状态栏常驻
RX / TX 字节数。

<!-- 界面截图：跑起来截一张图存为 docs/screenshot.png，然后把下面这行的注释去掉
<p align="center"><img src="docs/screenshot.png" width="760" alt="bypSerial 主界面"></p>
-->

---

## 使用说明

**连接串口**：点击「刷新」扫描端口 → 选中目标串口（如 COM3）→ 选波特率（默认 115200）→ 点「打开串口」。

**接收数据**：数据实时进入日志区；勾选「HEX 显示」按十六进制查看原始字节；取消「自动滚动」可停在当前位置翻阅。

**发送数据**：底部输入框输入内容 → 勾选「HEX 发送」时按 `FF 0A 1B` 格式填写 → 点「发送」或直接回车。

**保存日志**：点「保存日志」，当前会话全部收发记录导出为 TXT。

| 操作 | 快捷键 |
| --- | --- |
| 发送 | `Enter` |
| 使用说明 | `F1` |

---

## 项目结构

```text
bypSerial/
├── bypSerial.pro       # Qt 项目文件（版本号唯一来源）
├── main.cpp            # 程序入口，注入应用名与版本
├── mainwindow.h        # 主窗口声明
├── mainwindow.cpp      # 主窗口实现（控件、串口收发、日志）
├── mainwindow.ui       # 界面骨架（控件在 cpp 中构建）
├── bypSerial.qrc       # 资源文件（程序内图标）
├── bypSerial.png       # 应用图标（qrc 内嵌）
├── bypSerial.ico       # EXE 图标（RC_ICONS 内嵌）
├── build-release.ps1   # Windows 一键 Release 构建 + windeployqt + 打包
├── CHANGELOG.md        # 版本变更记录
├── LICENSE             # MIT
├── .gitignore          # 排除构建产物与 IDE 配置
└── README.md
```

---

## 依赖与验证情况

| 项 | 说明 |
| --- | --- |
| Qt | 6.x（`core`、`gui`、`widgets`、`serialport`），开发使用 6.12.0 |
| 编译器 | MinGW 13.1（GCC 64-bit） |
| 已验证平台 | Windows 11 |
| 未验证 | Linux / macOS 理论上可编译，仓库内 Linux、macOS 命令未经实测 |

---

## 已知限制

| 限制 | 影响 | 临时办法 |
| --- | --- | --- |
| 固定 8N1 | 无法连接 7E1 / 8E1 等非 8N1 设备 | 暂无 |
| 无 DTR / RTS 控制 | ESP32 自动下载、依赖信号复位的板子用不了 | 手动按 BOOT + RST |
| 无 `\r\n` 自动追加 | 部分 MCU 命令行需要行尾才会执行 | 用「HEX 发送」补 `0D 0A` |
| 文本模式按 `\n` 分帧 | 不带换行的数据不会即时成行显示 | 勾「HEX 显示」看原始字节 |
| 无换行数据超过 4 KB 强制输出 | 超长单行会被切成多段 | 同上 |

---

## 版本与发布

版本号只在 [bypSerial.pro](bypSerial.pro) 的 `VERSION` 一处维护。窗口标题、帮助与
关于对话框、exe 的文件版本信息、构建脚本产出的包名全部由它派生。

递增判据：

- **主版本**（`1.x` → `2.0`）：新增功能能力，如定时发送、多会话
- **次版本**（`1.0` → `1.1`）：界面重做、既有缺陷全修、工具链迁移等精修升级
- **修订号**（`1.0.0` → `1.0.1`）：少量单点修复，功能集合不变

发版流程：

1. 改 `bypSerial.pro` 的 `VERSION`
2. 在 [CHANGELOG.md](CHANGELOG.md) 里把「未发布」条目挪到新版本号下
3. 运行 `build-release.ps1`，得到 `build-release\release\` 与 `bypSerial-v<版本>-win64.zip`
4. `git tag -a v<版本> -m "bypSerial v<版本>"` → `git push --follow-tags`
5. GitHub Releases 上传 zip，正文贴 CHANGELOG 对应小节

完整变更历史见 [CHANGELOG.md](CHANGELOG.md)。

---

## 关于作者

`BYP Tools Series` 是白毅平的个人自用工具系列，命名统一 `byp` 前缀，按"够用就好、
离线优先"的原则写，成熟一个开源一个。

- Website：[byp.com.cn](https://byp.com.cn)
- GitHub：[@eebyp](https://github.com/eebyp)
- Hugging Face：[@eebyp](https://huggingface.co/eebyp)
- Email：[byp@byp.com.cn](mailto:byp@byp.com.cn)

## 许可证

[MIT License](LICENSE) © 2026 白毅平

---

<sub>**English** — bypSerial is a lightweight Qt 6 serial port monitor for embedded
debugging: port enumeration, text/HEX RX & TX, byte counters and log export.
Prebuilt Windows binaries are attached to
[Releases](https://github.com/eebyp/bypSerial/releases/latest).</sub>

<sub>版本 1.0.0（以 `bypSerial.pro` 的 `VERSION` 为准）· 更新于 2026-11-08</sub>
