#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDateTime>
#include <QFileDialog>
#include <QTextStream>
#include <QMenuBar>
#include <QAction>
#include <QIcon>
#include <QDesktopServices>
#include <QUrl>
#include <QCoreApplication>

namespace {
// 文本模式下等待换行符的缓冲上限：对端长时间不发 '\n' 时防止内存无界增长
const int kMaxPendingLine = 4096;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , serial(new QSerialPort(this))
    , isConnected(false)
    , rxBytes(0)
    , txBytes(0)
{
    ui->setupUi(this);
    setWindowTitle("bypSerial - 串口监视器 v1.0.0");
    
    // 窗口/任务栏图标：从编译进 exe 的 Qt 资源取，不依赖外部文件路径
    QIcon appIcon(":/bypSerial.png");
    if (!appIcon.isNull()) {
        setWindowIcon(appIcon);
    }
    
    resize(800, 600);

    // 创建菜单栏
    createMenuBar();

    // 创建控件
    portCombo = new QComboBox;
    baudCombo = new QComboBox;
    baudCombo->addItems({"9600", "19200", "38400", "57600", "115200", "230400", "460800", "921600"});
    baudCombo->setCurrentText("115200");
    baudCombo->setEditable(true);
    
    refreshBtn = new QPushButton("🔄 刷新");
    connectBtn = new QPushButton("打开串口");
    sendBtn = new QPushButton("发送");
    clearBtn = new QPushButton("清空");
    saveBtn = new QPushButton("保存日志");
    
    sendEdit = new QLineEdit;
    sendEdit->setPlaceholderText("输入要发送的数据...");
    
    logText = new QTextEdit;
    logText->setReadOnly(true);
    logText->setFont(QFont("Consolas", 10));
    
    hexRxCheck = new QCheckBox("HEX 显示");
    hexTxCheck = new QCheckBox("HEX 发送");
    autoScrollCheck = new QCheckBox("自动滚动");
    autoScrollCheck->setChecked(true);
    
    statusLabel = new QLabel("未连接");
    rxLabel = new QLabel("RX: 0 B");
    txLabel = new QLabel("TX: 0 B");

    // 布局
    QWidget *central = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    
    // 顶部：串口配置
    QGroupBox *configGroup = new QGroupBox("串口配置");
    QHBoxLayout *configLayout = new QHBoxLayout(configGroup);
    configLayout->addWidget(new QLabel("串口:"));
    configLayout->addWidget(portCombo, 1);
    configLayout->addWidget(refreshBtn);
    configLayout->addWidget(new QLabel("波特率:"));
    configLayout->addWidget(baudCombo);
    configLayout->addWidget(connectBtn);
    mainLayout->addWidget(configGroup);
    
    // 中部：日志显示
    QGroupBox *logGroup = new QGroupBox("数据日志");
    QVBoxLayout *logLayout = new QVBoxLayout(logGroup);
    logLayout->addWidget(logText);
    
    QHBoxLayout *logBtnLayout = new QHBoxLayout;
    logBtnLayout->addWidget(hexRxCheck);
    logBtnLayout->addWidget(autoScrollCheck);
    logBtnLayout->addStretch();
    logBtnLayout->addWidget(clearBtn);
    logBtnLayout->addWidget(saveBtn);
    logLayout->addLayout(logBtnLayout);
    mainLayout->addWidget(logGroup, 1);
    
    // 底部：发送区
    QGroupBox *sendGroup = new QGroupBox("发送数据");
    QVBoxLayout *sendLayout = new QVBoxLayout(sendGroup);
    
    QHBoxLayout *sendEditLayout = new QHBoxLayout;
    sendEditLayout->addWidget(sendEdit, 1);
    sendEditLayout->addWidget(hexTxCheck);
    sendEditLayout->addWidget(sendBtn);
    sendLayout->addLayout(sendEditLayout);
    mainLayout->addWidget(sendGroup);
    
    // 状态栏
    QStatusBar *statusBar = new QStatusBar;
    statusBar->addWidget(statusLabel);
    statusBar->addPermanentWidget(rxLabel);
    statusBar->addPermanentWidget(txLabel);
    setStatusBar(statusBar);
    
    setCentralWidget(central);

    // 信号槽
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::onRefreshPorts);
    connect(connectBtn, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    connect(serial, &QSerialPort::readyRead, this, &MainWindow::onReadData);
    connect(sendBtn, &QPushButton::clicked, this, &MainWindow::onSendClicked);
    connect(clearBtn, &QPushButton::clicked, this, &MainWindow::onClearLog);
    connect(saveBtn, &QPushButton::clicked, this, &MainWindow::onSaveLog);
    connect(sendEdit, &QLineEdit::returnPressed, this, &MainWindow::onSendClicked);
    connect(serial, &QSerialPort::errorOccurred, this, &MainWindow::onSerialError);
    
    // 初始化
    onRefreshPorts();
    appendLog("=== bypSerial 串口监视器已启动 ===", Qt::darkGreen);
    appendLog("请选择串口并点击'打开串口'开始监视", Qt::gray);
}

MainWindow::~MainWindow()
{
    if (serial->isOpen()) {
        serial->close();
    }
    delete ui;
}

void MainWindow::onRefreshPorts()
{
    updatePortList();
    appendLog("已刷新串口列表", Qt::gray);
}

void MainWindow::updatePortList()
{
    portCombo->clear();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        QString text = info.portName();
        if (!info.description().isEmpty()) {
            text += " - " + info.description();
        }
        portCombo->addItem(text, info.portName());
    }
    
    if (portCombo->count() == 0) {
        portCombo->addItem("(未找到串口)");
    }
}

void MainWindow::onConnectClicked()
{
    if (isConnected) {
        // 关闭串口
        serial->close();
        rxBuffer.clear();  // 丢弃未完成的半行，避免残留数据串到下一次会话
        applyConnectionState(false);
        appendLog("串口已关闭", Qt::darkRed);
    } else {
        // 打开串口
        QString portName = portCombo->currentData().toString();
        if (portName.isEmpty()) {
            QMessageBox::warning(this, "警告", "请选择有效的串口");
            return;
        }
        
        serial->setPortName(portName);
        serial->setBaudRate(baudCombo->currentText().toInt());
        serial->setDataBits(QSerialPort::Data8);
        serial->setParity(QSerialPort::NoParity);
        serial->setStopBits(QSerialPort::OneStop);
        serial->setFlowControl(QSerialPort::NoFlowControl);
        
        if (serial->open(QIODevice::ReadWrite)) {
            applyConnectionState(true);
            statusLabel->setText(QString("已连接: %1 @ %2").arg(portName).arg(baudCombo->currentText()));
            appendLog(QString("串口已打开: %1 @ %2 bps").arg(portName).arg(baudCombo->currentText()), Qt::darkGreen);
            
            rxBytes = 0;
            txBytes = 0;
            rxLabel->setText("RX: 0 B");
            txLabel->setText("TX: 0 B");
        } else {
            QMessageBox::critical(this, "错误", 
                QString("无法打开串口 %1\n\n错误信息: %2").arg(portName).arg(serial->errorString()));
        }
    }
}

void MainWindow::applyConnectionState(bool connected)
{
    isConnected = connected;
    connectBtn->setText(connected ? "关闭串口" : "打开串口");
    connectBtn->setStyleSheet(connected ? "background-color: #ff6b6b; color: white;" : "");
    if (!connected) {
        statusLabel->setText("未连接");
    }
}

void MainWindow::onSerialError(QSerialPort::SerialPortError error)
{
    // 打开串口失败由 onConnectClicked() 自己提示，这里只处理连接建立后的中断
    if (error == QSerialPort::NoError || !isConnected) {
        return;
    }

    // 设备被拔出或驱动句柄失效：串口已无法再读写，必须把界面回滚到未连接状态，
    // 否则界面仍显示"已连接"，用户要等到下次点发送才会发现
    if (error == QSerialPort::ResourceError || error == QSerialPort::PermissionError) {
        const QString reason = serial->errorString();
        // 先改状态再 close()：避免 close() 重入触发 errorOccurred 时仍被判为“已连接”
        applyConnectionState(false);
        serial->close();
        rxBuffer.clear();
        appendLog(QString("串口已断开: ") + reason, Qt::darkRed);
        QMessageBox::warning(this, "串口断开",
            QString("串口连接已中断，可能是设备被拔出或驱动异常。\n\n错误信息: %1").arg(reason));
        return;
    }

    appendLog(QString("串口错误: ") + serial->errorString(), Qt::darkRed);
}

void MainWindow::onReadData()
{
    const QByteArray data = serial->readAll();
    if (data.isEmpty()) {
        return;
    }

    rxBytes += data.size();
    rxLabel->setText(QString("RX: %1 B").arg(rxBytes));

    // HEX 模式：不按行切分，整块字节立即输出。
    // 二进制帧/定长帧里没有 '\n'，若走按行缓冲会一直卡在缓冲区不显示。
    if (hexRxCheck->isChecked()) {
        appendRx(data.toHex(' ').toUpper());
        return;
    }

    // 文本模式：按 '\n' 分行，行内空白与空行原样保留（监视器不改写收到的内容）
    rxBuffer.append(data);
    while (true) {
        const int newlinePos = rxBuffer.indexOf('\n');
        if (newlinePos < 0) {
            break;
        }

        QByteArray line = rxBuffer.left(newlinePos);
        rxBuffer.remove(0, newlinePos + 1);

        // 只去掉行尾的 \r，不做 trimmed：缩进与对齐空格也是数据的一部分
        if (line.endsWith('\r')) {
            line.chop(1);
        }

        appendRx(QString::fromUtf8(line));
    }

    // 对端长时间不发 '\n' 时强制输出，避免缓冲区无界增长
    if (rxBuffer.size() > kMaxPendingLine) {
        appendRx(QString::fromUtf8(rxBuffer));
        rxBuffer.clear();
    }
}

void MainWindow::onSendClicked()
{
    if (!isConnected) {
        QMessageBox::warning(this, "警告", "请先打开串口");
        return;
    }
    
    QString text = sendEdit->text();
    if (text.isEmpty()) return;
    
    QByteArray data;
    if (hexTxCheck->isChecked()) {
        // HEX 发送
        data = QByteArray::fromHex(text.remove(' ').toUtf8());
    } else {
        // 文本发送
        data = text.toUtf8();
    }
    
    qint64 written = serial->write(data);
    if (written > 0) {
        txBytes += written;
        txLabel->setText(QString("TX: %1 B").arg(txBytes));
        appendTx(text);
        sendEdit->clear();
    } else {
        appendLog("发送失败: " + serial->errorString(), Qt::darkRed);
    }
}

void MainWindow::onClearLog()
{
    logText->clear();
    rxBuffer.clear();
    rxBytes = 0;
    txBytes = 0;
    rxLabel->setText("RX: 0 B");
    txLabel->setText("TX: 0 B");
}

void MainWindow::onSaveLog()
{
    QString filename = QFileDialog::getSaveFileName(this, "保存日志", 
        QString("bypSerial_log_%1.txt").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        "文本文件 (*.txt);;所有文件 (*)");
    
    if (!filename.isEmpty()) {
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << logText->toPlainText();
            file.close();
            appendLog(QString("日志已保存: %1").arg(filename), Qt::darkGreen);
        }
    }
}

void MainWindow::appendLog(const QString &text, const QColor &color)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    logText->setTextColor(color);
    logText->append(QString("[%1] %2").arg(timestamp).arg(text));
    
    if (autoScrollCheck->isChecked()) {
        logText->moveCursor(QTextCursor::End);
    }
}

void MainWindow::appendRx(const QString &text)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    logText->setTextColor(Qt::blue);
    logText->append(QString("[%1] RX: %2").arg(timestamp).arg(text));
    
    if (autoScrollCheck->isChecked()) {
        logText->moveCursor(QTextCursor::End);
    }
}

void MainWindow::appendTx(const QString &text)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    logText->setTextColor(Qt::darkMagenta);
    logText->append(QString("[%1] TX: %2").arg(timestamp).arg(text));
    
    if (autoScrollCheck->isChecked()) {
        logText->moveCursor(QTextCursor::End);
    }
}

void MainWindow::createMenuBar()
{
    mainMenuBar = this->menuBar();
    
    // 帮助菜单
    helpMenu = mainMenuBar->addMenu("帮助 (&H)");
    
    helpAction = helpMenu->addAction("使用说明 (&F1)");
    helpAction->setShortcut(QKeySequence::HelpContents);
    connect(helpAction, &QAction::triggered, this, &MainWindow::onHelp);
    
    helpMenu->addSeparator();
    
    aboutAction = helpMenu->addAction("关于 bypSerial (&A)");
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::onHelp()
{
    QMessageBox::information(this, "bypSerial 使用说明",
        "<h3>bypSerial 串口监视器 v1.0.0</h3>"
        "<p><b>BYP 工具系列</b> - 嵌入式开发调试工具</p>"
        "<hr>"
        "<h4>基本操作</h4>"
        "<ul>"
        "<li><b>刷新串口</b>：扫描系统可用的 COM 端口</li>"
        "<li><b>打开/关闭串口</b>：连接或断开串口</li>"
        "<li><b>波特率</b>：支持 9600 ~ 921600，也可手动输入</li>"
        "</ul>"
        "<h4>数据收发</h4>"
        "<ul>"
        "<li><b>HEX 显示</b>：以十六进制格式显示接收数据</li>"
        "<li><b>HEX 发送</b>：以十六进制格式发送数据（如 FF 0A 1B）</li>"
        "<li><b>自动滚动</b>：新数据到达时自动滚动到底部</li>"
        "</ul>"
        "<h4>日志管理</h4>"
        "<ul>"
        "<li><b>清空</b>：清除当前显示的所有日志</li>"
        "<li><b>保存日志</b>：将收发记录导出为 TXT 文件</li>"
        "</ul>"
        "<hr>"
        "<p><i>提示：按 F1 可随时查看此帮助</i></p>",
        QMessageBox::Ok);
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "关于 bypSerial",
        "<h2>bypSerial v1.0.0</h2>"
        "<p><b>串口监视器</b></p>"
        "<p>BYP 工具系列 - 嵌入式开发调试工具</p>"
        "<hr>"
        "<p><b>功能特性</b></p>"
        "<ul>"
        "<li>自动枚举可用串口</li>"
        "<li>支持常用波特率（9600 ~ 921600）</li>"
        "<li>实时接收数据显示（文本/HEX）</li>"
        "<li>数据发送（文本/HEX）</li>"
        "<li>收发字节统计</li>"
        "<li>日志保存功能</li>"
        "</ul>"
        "<hr>"
        "<p><b>技术栈</b></p>"
        "<p>Qt 6.x + QSerialPort</p>"
        "<hr>"
        "<p style='color: gray;'>© 2026 BYP Tools Series</p>");
}
