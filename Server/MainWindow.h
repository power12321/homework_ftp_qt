#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QLineEdit;
class QPushButton;
class QTextEdit;
class WinsockServer;

/**
 * 主窗口：输入端口、Start/Stop 服务器，并显示日志
 */
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onStartServer();
    void onStopServer();
    void appendLog(const QString &msg);

private:
    QLineEdit      *editPort_;
    QPushButton    *btnStart_, *btnStop_;
    QTextEdit      *logArea_;
    WinsockServer  *server_;
};

#endif // MAINWINDOW_H
