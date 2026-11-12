# bypSerial Release 构建脚本
# 使用方法：右键 → 使用 PowerShell 运行

$ErrorActionPreference = "Stop"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  bypSerial Release 构建脚本" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# 1. 创建 release 构建目录
if (!(Test-Path "build-release")) {
    New-Item -ItemType Directory -Path "build-release" | Out-Null
}
Set-Location build-release

# 2. 运行 qmake（Release 模式）
Write-Host "[1/4] 运行 qmake (Release 模式)..." -ForegroundColor Yellow
qmake ../bypSerial.pro CONFIG+=release

# 3. 编译
# ico 变更后 windres 不会自动重跑：Makefile 的资源目标只依赖生成的 .rc，
# 而 .rc 内容（图标路径）不变，故先删掉旧的 _resource_res.o 强制重编
Write-Host "[2/4] 编译中..." -ForegroundColor Yellow
if (Test-Path "release\bypSerial_resource_res.o") {
    Remove-Item "release\bypSerial_resource_res.o" -Force
}
mingw32-make -j4

# 4. 检查编译结果
if (!(Test-Path "release\bypSerial.exe")) {
    Write-Host "错误：编译失败！" -ForegroundColor Red
    exit 1
}

Write-Host "[3/4] 编译成功！" -ForegroundColor Green

# 5. 打包 Qt 依赖
Write-Host "[4/4] 打包 Qt 依赖..." -ForegroundColor Yellow
windeployqt release\bypSerial.exe

# 6. 图标说明：bypSerial.ico 由 RC_ICONS 内嵌进 exe，bypSerial.png 由 qrc 打进程序内，
#    两者都不需要再往发布目录复制外部文件
#    重新生成图标：D:\B\software\anaconda\python.exe ..\QtIconMaker\make_icon.py tools\icon_src.png bypSerial --preview

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "  构建完成！" -ForegroundColor Green
Write-Host "  输出目录: build-release\release\" -ForegroundColor Green
Write-Host "  可执行文件: bypSerial.exe" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "提示：可以将 release 文件夹复制到 d:\BYP\Tools\bypSerial\ 作为独立工具使用" -ForegroundColor Cyan
