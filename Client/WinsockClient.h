#ifndef WINSOCKCLIENT_H
#define WINSOCKCLIENT_H

#include <QObject>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <atomic>

/**
 * @brief 基于 Winsock 的简单 FTP 客户端
 * 本地所有文件读写都在 ./file 子目录下完成
 */
class WinsockClient : public QObject {
    Q_OBJECT
public:
    explicit WinsockClient(QObject *parent = nullptr);
    ~WinsockClient() override;

    /// 连接到服务器
    void connectToHost(const QString &host, quint16 port);
    /// 断开连接
    void disconnectFromHost();
    /// 发送一行 FTP 命令（LIST, GET, PUT, QUIT）
    void sendCommand(const QString &cmdLine);

signals:
    /// 日志输出到 UI
    void logMessage(const QString &msg);

private:
    SOCKET            sock_{INVALID_SOCKET};
    std::atomic<bool> connected_{false};
};


#endif // WINSOCKCLIENT_H
