#ifndef WINSOCKSERVER_H
#define WINSOCKSERVER_H

#include <QObject>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <atomic>

/**
 * @brief 基于 Winsock 的多客户端 FTP Server
 *
 * 支持的命令：
 *   LIST       列出 ./file 目录下的所有文件
 *   GET <f>    从 ./file/<f> 下载文件
 *   PUT <f>    上传并保存为 ./file/<f>
 *   QUIT       关闭当前客户端连接
 */
class WinsockServer : public QObject {
    Q_OBJECT
public:
    explicit WinsockServer(QObject *parent = nullptr);
    ~WinsockServer() override;

    /// 在给定端口启动监听
    void start(quint16 port);
    /// 停止监听并关闭所有连接
    void stop();

signals:
    /// 日志消息，传给 UI 显示
    void logMessage(const QString &msg);

private:
    SOCKET            listenSock_{INVALID_SOCKET};  ///< 监听套接字
    std::thread       acceptThread_;                ///< 接受连接线程
    std::atomic<bool> running_{false};              ///< 运行状态

    /// 接受连接的循环函数
    void acceptLoop();
    /// 处理单个客户端会话
    void handleClient(SOCKET clientSock);
};


#endif // WINSOCKSERVER_H
