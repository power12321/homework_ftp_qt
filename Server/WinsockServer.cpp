#include "WinsockServer.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QString>
#include <sstream>

WinsockServer::WinsockServer(QObject *parent)
    : QObject(parent)
{}

WinsockServer::~WinsockServer()
{
    stop();
}

void WinsockServer::start(quint16 port)
{
    if (running_) {
        emit logMessage("Server already running");
        return;
    }

    // —— 确保 ./file 目录存在，否则创建它 ——
    {
        QDir dir;
        if (!dir.exists("file")) {
            if (dir.mkpath("file")) {
                emit logMessage("Created directory './file'");
            } else {
                emit logMessage("ERROR: failed to create './file'");
                return;
            }
        }
    }

    // 1) 创建 TCP socket
    listenSock_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSock_ == INVALID_SOCKET) {
        emit logMessage("socket() failed");
        return;
    }

    // 2) 绑定到本地任意地址与指定端口
    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(port);
    if (::bind(listenSock_, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        emit logMessage("bind() failed");
        ::closesocket(listenSock_);
        return;
    }

    // 3) 开始监听
    if (::listen(listenSock_, SOMAXCONN) == SOCKET_ERROR) {
        emit logMessage("listen() failed");
        ::closesocket(listenSock_);
        return;
    }

    running_ = true;
    emit logMessage(QString("Server listening on port %1").arg(port));

    // 4) 启动后台线程接受连接
    acceptThread_ = std::thread(&WinsockServer::acceptLoop, this);
}

void WinsockServer::stop()
{
    if (!running_) return;
    running_ = false;
    ::closesocket(listenSock_);
    if (acceptThread_.joinable())
        acceptThread_.join();
    emit logMessage("Server stopped");
}

void WinsockServer::acceptLoop()
{
    while (running_) {
        sockaddr_in clientAddr;
        int len = sizeof(clientAddr);
        SOCKET clientSock = ::accept(listenSock_, (sockaddr*)&clientAddr, &len);
        if (clientSock == INVALID_SOCKET) {
            if (running_) emit logMessage("accept() failed");
            break;
        }
        QString peer = inet_ntoa(clientAddr.sin_addr);
        emit logMessage(QString("Client connected: %1").arg(peer));
        std::thread(&WinsockServer::handleClient, this, clientSock).detach();
    }
}

void WinsockServer::handleClient(SOCKET clientSock)
{
    auto sendAll = [&](const std::string &data) {
        int total = 0, n = int(data.size());
        while (total < n) {
            int sent = ::send(clientSock, data.c_str() + total, n - total, 0);
            if (sent == SOCKET_ERROR) return false;
            total += sent;
        }
        return true;
    };

    char buffer[4096];
    std::string readBuf;

    while (true) {
        int received = ::recv(clientSock, buffer, sizeof(buffer), 0);
        if (received <= 0) break;
        readBuf.append(buffer, received);

        size_t pos;
        while ((pos = readBuf.find('\n')) != std::string::npos) {
            std::string line = readBuf.substr(0, pos);
            readBuf.erase(0, pos + 1);
            if (!line.empty() && line.back() == '\r')
                line.pop_back();

            emit logMessage(">> " + QString::fromStdString(line));

            std::istringstream iss(line);
            std::string op; iss >> op;
            for (auto &c : op) c = toupper(c);

            if (op == "LIST") {
                // 列出 ./file 目录下的所有普通文件
                QDir dir("./file");
                if (!dir.exists()) {
                    sendAll("ERROR: directory './file' not found\n");
                    sendAll("END_OF_LIST\n");
                } else {
                    QStringList files = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
                    for (const QString &f : files) {
                        sendAll(f.toStdString() + "\n");
                    }
                    sendAll("END_OF_LIST\n");
                }
            }
            else if (op == "GET") {
                // GET <filename>
                std::string fname; iss >> fname;
                QString path = QDir("./file").filePath(QString::fromStdString(fname));
                QFile file(path);
                if (!file.open(QIODevice::ReadOnly)) {
                    sendAll("ERROR: cannot open file\n");
                    continue;
                }
                qint64 sz = file.size();
                sendAll("SIZE " + std::to_string(sz) + "\n");
                int r = ::recv(clientSock, buffer, sizeof(buffer), 0);
                std::string ack(buffer, r);
                if (ack.find("READY") == std::string::npos) {
                    file.close();
                    continue;
                }
                while (!file.atEnd()) {
                    QByteArray chunk = file.read(4096);
                    ::send(clientSock, chunk.constData(), chunk.size(), 0);
                }
                file.close();
            }
            else if (op == "PUT") {
                // PUT <filename>
                std::string fname; iss >> fname;
                sendAll("OK\n");
                int r = ::recv(clientSock, buffer, sizeof(buffer), 0);
                std::string sizeLine(buffer, r);
                qint64 sz = std::stoll(sizeLine.substr(5));
                sendAll("READY\n");
                QString path = QDir("./file").filePath(QString::fromStdString(fname));
                QFile file(path);
                if (!file.open(QIODevice::WriteOnly)) break;
                qint64 recvd = 0;
                while (recvd < sz) {
                    r = ::recv(clientSock, buffer, sizeof(buffer), 0);
                    if (r <= 0) break;
                    file.write(buffer, r);
                    recvd += r;
                }
                file.close();
            }
            else if (op == "QUIT") {
                ::closesocket(clientSock);
                emit logMessage("Client disconnected");
                return;
            }
            else {
                sendAll("ERROR: unknown command\n");
            }
        }
    }

    ::closesocket(clientSock);
    emit logMessage("Client disconnected");
}
