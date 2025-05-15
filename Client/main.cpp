#include <winsock2.h>
#include <ws2tcpip.h>
#include <QApplication>
#include <QMessageBox>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    // 声明一个 WSADATA 结构体，用于接收 WSAStartup 的初始化信息
    WSADATA wsaData;

    // 初始化 Winsock 2.2 库
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        QMessageBox::critical(nullptr, "Error", "WSAStartup failed");
        return -1;
    }

    // 启动 Qt 应用
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    int ret = app.exec();

    // 清理 Winsock
    WSACleanup();
    return ret;
}
