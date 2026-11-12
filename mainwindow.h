#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStatusBar>
#include <QMenuBar>
#include <QMenu>
#include <QAction>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onRefreshPorts();
    void onConnectClicked();
    void onReadData();
    void onSendClicked();
    void onClearLog();
    void onSaveLog();
    void onHelp();
    void onAbout();
    void onSerialError(QSerialPort::SerialPortError error);

private:
    void updatePortList();
    void createMenuBar();
    void applyConnectionState(bool connected);
    void appendLog(const QString &text, const QColor &color);
    void appendRx(const QString &text);
    void appendTx(const QString &text);

    Ui::MainWindow *ui;
    QSerialPort *serial;
    
    // 控件
    QComboBox *portCombo;
    QComboBox *baudCombo;
    QPushButton *refreshBtn;
    QPushButton *connectBtn;
    QPushButton *sendBtn;
    QPushButton *clearBtn;
    QPushButton *saveBtn;
    QLineEdit *sendEdit;
    QTextEdit *logText;
    QCheckBox *hexRxCheck;
    QCheckBox *hexTxCheck;
    QCheckBox *autoScrollCheck;
    QLabel *statusLabel;
    QLabel *rxLabel;
    QLabel *txLabel;
    
    // 状态
    bool isConnected;
    qint64 rxBytes;
    qint64 txBytes;
    QByteArray rxBuffer;  // 接收缓冲区（按行处理）
    
    // 菜单栏
    QMenuBar *mainMenuBar;
    QMenu *helpMenu;
    QAction *aboutAction;
    QAction *helpAction;
};

#endif // MAINWINDOW_H
