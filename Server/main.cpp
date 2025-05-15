#include <winsock2.h>
#include <ws2tcpip.h>
#include <QApplication>
#include <QMessageBox>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    // 1) 初始化 Winsock
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        QMessageBox::critical(nullptr, "Error", "WSAStartup failed");
        return -1;
    }

    // 2) 启动 Qt 应用
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    int ret = app.exec();

    // 3) 清理 Winsock
    WSACleanup();
    return ret;
}
