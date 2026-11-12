QT += core gui serialport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET = bypSerial
TEMPLATE = app

# 版本号单一来源：窗口标题、帮助/关于对话框、exe 文件版本信息、发布包名都从这里派生
# 只改这一处；CHANGELOG.md 同步加一节
VERSION = 1.0.0
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

RESOURCES += \
    bypSerial.qrc

# EXE 文件图标：qmake 生成 app_res.rc 后调 windres 编译，由链接器内嵌进 exe 的 .rsrc 节
RC_ICONS = bypSerial.ico
