#include "mainwindow.h"
#include <QApplication>
#include <QCoreApplication>

// 由 bypSerial.pro 的 DEFINES 注入；非 qmake 构建时保底，保证仍可编译
#ifndef APP_VERSION
#define APP_VERSION "0.0.0-dev"
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("bypSerial"));
    QCoreApplication::setApplicationVersion(QStringLiteral(APP_VERSION));

    // 设置应用样式
    a.setStyle("Fusion");
    
    MainWindow w;
    w.show();
    
    return a.exec();
}
