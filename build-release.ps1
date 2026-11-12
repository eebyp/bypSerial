# bypSerial Release 构建脚本
# 使用方法：右键 → 使用 PowerShell 运行

$ErrorActionPreference = "Stop"

# 0. 读取版本号：单一来源为 bypSerial.pro 的 VERSION
$proText = Get-Content "bypSerial.pro" -Raw
if ($proText -notmatch '(?m)^\s*VERSION\s*=\s*([0-9][0-9.]*)') {
    Write-Host "错误：无法从 bypSerial.pro 读取 VERSION" -ForegroundColor Red
    exit 1
}
$version = $Matches[1]

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  bypSerial Release 构建脚本" -ForegroundColor Cyan
Write-Host "  版本: v$version" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# 1. 创建 release 构建目录
if (!(Test-Path "build-release")) {
    New-Item -ItemType Directory -Path "build-release" | Out-Null
}
Set-Location build-release

# 2. 运行 qmake（Release 模式）
Write-Host "[1/5] 运行 qmake (Release 模式)..." -ForegroundColor Yellow
qmake ../bypSerial.pro CONFIG+=release

# 3. 编译
# ico 变更后 windres 不会自动重跑：Makefile 的资源目标只依赖生成的 .rc，
# 而 .rc 内容（图标路径）不变，故先删掉旧的 _resource_res.o 强制重编
Write-Host "[2/5] 编译中..." -ForegroundColor Yellow
if (Test-Path "release\bypSerial_resource_res.o") {
    Remove-Item "release\bypSerial_resource_res.o" -Force
}
mingw32-make -j4

# 4. 检查编译结果
if (!(Test-Path "release\bypSerial.exe")) {
    Write-Host "错误：编译失败！" -ForegroundColor Red
    exit 1
}

Write-Host "[3/5] 编译成功！" -ForegroundColor Green

# 5. 打包 Qt 依赖
# windeployqt 会把 dxcompiler.dll / dxil.dll 这类无害警告写进 stderr，
# 在 $ErrorActionPreference = "Stop" 下会被当成终止错误，故临时放宽再用产物校验保底
Write-Host "[4/5] 打包 Qt 依赖..." -ForegroundColor Yellow
$prevEap = $ErrorActionPreference
$ErrorActionPreference = "Continue"
windeployqt release\bypSerial.exe
$ErrorActionPreference = $prevEap
if (!(Test-Path "release\Qt6Core.dll")) {
    Write-Host "错误：windeployqt 未生成 Qt 依赖，发布包不完整！" -ForegroundColor Red
    exit 1
}

# 6. 图标说明：bypSerial.ico 由 RC_ICONS 内嵌进 exe，bypSerial.png 由 qrc 打进程序内，
#    两者都不需要再往发布目录复制外部文件
#    重新生成图标：python ..\QtIconMaker\make_icon.py tools\icon_src.png bypSerial --preview

# 7. 打包发布 zip：文件名带版本号；同名产物已存在时追加时间戳，避免覆盖历史产物
$zipName = "bypSerial-v$version-win64.zip"
if (Test-Path $zipName) {
    $zipName = "bypSerial-v$version-win64_$(Get-Date -Format 'yyyyMMdd_HHmmss').zip"
}
Write-Host "[5/5] 打包发布包..." -ForegroundColor Yellow
Compress-Archive -Path "release\*" -DestinationPath $zipName -CompressionLevel Optimal

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "  构建完成！ v$version" -ForegroundColor Green
Write-Host "  输出目录: build-release\release\" -ForegroundColor Green
Write-Host "  可执行文件: bypSerial.exe" -ForegroundColor Green
Write-Host "  发布包: build-release\$zipName" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "提示：可以将 release 文件夹复制到工具目录（如 D:\Tools\bypSerial\）作为独立工具使用" -ForegroundColor Cyan
