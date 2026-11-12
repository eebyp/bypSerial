QT += core gui serialport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET = bypSerial
TEMPLATE = app

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
