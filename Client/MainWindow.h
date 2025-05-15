#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QLineEdit;
class QPushButton;
class QTextEdit;
class WinsockClient;

/**
 * 主窗口：输入 IP/Port，Connect/Disconnect，输入命令 Send，并显示日志
 */
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onConnect();
    void onDisconnect();
    void onSend();
    void appendLog(const QString &msg);

private:
    QLineEdit     *editIp_, *editPort_, *editCmd_;
    QPushButton   *btnConnect_, *btnDisconnect_, *btnSend_;
    QTextEdit     *logArea_;
    WinsockClient *client_;
};

#endif // MAINWINDOW_H
