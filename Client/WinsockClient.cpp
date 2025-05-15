#include "WinsockClient.h"
#include <QString>
#include <QDir>
#include <thread>
#include <fstream>
#include <sstream>

WinsockClient::WinsockClient(QObject *parent)
    : QObject(parent)
{
    // 确保本地 ./file 目录存在
    QDir dir;
    if (!dir.exists("file")) {
        if (dir.mkpath("file")) {
            emit logMessage("Created local directory './file'");
        } else {
            emit logMessage("ERROR: failed to create local './file'");
        }
    }
}

WinsockClient::~WinsockClient()
{
    disconnectFromHost();
}

void WinsockClient::connectToHost(const QString &host, quint16 port)
{
    if (connected_) return;

    // 创建 socket
    sock_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock_ == INVALID_SOCKET) {
        emit logMessage("socket() failed");
        return;
    }

    // 准备地址
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, host.toStdString().c_str(), &addr.sin_addr);
    addr.sin_port = htons(port);

    // 强制调用全局 ::connect
    if (::connect(sock_,
                  reinterpret_cast<const sockaddr*>(&addr),
                  static_cast<int>(sizeof(addr))) == SOCKET_ERROR)
    {
        emit logMessage("connect() failed");
        ::closesocket(sock_);
        return;
    }

    connected_ = true;
    emit logMessage(QString("Connected to %1:%2")
                        .arg(host).arg(port));
}

void WinsockClient::disconnectFromHost()
{
    if (!connected_) return;
    ::closesocket(sock_);
    connected_ = false;
    emit logMessage("Disconnected");
}

void WinsockClient::sendCommand(const QString &cmdLine)
{
    if (!connected_) {
        emit logMessage("Not connected");
        return;
    }

    // 将阻塞操作移到后台线程，避免卡 UI
    std::string line = cmdLine.toStdString();
    std::thread([this, line]() {
        // Helper：可靠发送完整字符串
        auto sendAll = [&](const std::string &s) {
            int total = 0, n = int(s.size());
            while (total < n) {
                int sent = ::send(sock_, s.c_str() + total, n - total, 0);
                if (sent == SOCKET_ERROR) return false;
                total += sent;
            }
            return true;
        };

        std::istringstream iss(line);
        std::string op; iss >> op;
        for (auto &c : op) c = toupper(c);

        // ------------ LIST ------------
        if (op == "LIST") {
            sendAll(line + "\n");
            // 接收直到 END_OF_LIST
            char buf[4096]; std::string bufStr;
            while (true) {
                int r = ::recv(sock_, buf, sizeof(buf), 0);
                if (r <= 0) break;
                bufStr.append(buf, r);
                size_t pos;
                while ((pos = bufStr.find('\n')) != std::string::npos) {
                    std::string item = bufStr.substr(0, pos);
                    bufStr.erase(0, pos + 1);
                    if (item == "END_OF_LIST") return;
                    emit logMessage(QString::fromStdString(item));
                }
            }
        }
        // ------------ GET ------------
        else if (op == "GET") {
            // 语法：GET <remote> [local]
            std::string remote, local;
            iss >> remote >> local;
            if (local.empty()) local = remote;

            // 发送请求
            sendAll("GET " + remote + "\n");

            // 接收 SIZE
            char buf[64];
            int r = ::recv(sock_, buf, sizeof(buf), 0);
            std::string sizeLine(buf, r);
            if (sizeLine.rfind("SIZE", 0) != 0) {
                emit logMessage(QString::fromStdString(sizeLine));
                return;
            }
            uint64_t sz = std::stoull(sizeLine.substr(5));

            // 通知 READY
            sendAll("READY\n");

            // 打开本地文件 ./file/local
            QString qlocal = QDir("file").filePath(QString::fromStdString(local));
            std::ofstream ofs(qlocal.toStdString(), std::ios::binary);
            uint64_t recvd = 0;
            while (recvd < sz) {
                int b = ::recv(sock_, buf, sizeof(buf), 0);
                if (b <= 0) break;
                ofs.write(buf, b);
                recvd += b;
            }
            ofs.close();
            emit logMessage(QString("Downloaded %1 bytes to '%2'")
                                .arg(recvd)
                                .arg(qlocal));
        }
        // ------------ PUT ------------
        else if (op == "PUT") {
            // 语法：PUT <local> [remote]
            std::string local, remote;
            iss >> local >> remote;
            if (remote.empty()) remote = local;

            // 打开本地文件 ./file/local
            QString qlocal = QDir("file").filePath(QString::fromStdString(local));
            std::ifstream ifs(qlocal.toStdString(), std::ios::binary);
            if (!ifs) {
                emit logMessage(QString("Cannot open '%1'").arg(qlocal));
                return;
            }

            // 发送 PUT 命令
            sendAll("PUT " + remote + "\n");

            // 接收 OK
            char buf[64];
            int r = ::recv(sock_, buf, sizeof(buf), 0);
            std::string ok(buf, r);
            if (ok.rfind("OK", 0) != 0) {
                emit logMessage(QString::fromStdString(ok));
                return;
            }

            // 发送 SIZE
            ifs.seekg(0, std::ios::end);
            uint64_t sz = ifs.tellg();
            ifs.seekg(0);
            sendAll("SIZE " + std::to_string(sz) + "\n");

            // 等待 READY
            r = ::recv(sock_, buf, sizeof(buf), 0);
            std::string rd(buf, r);
            if (rd.rfind("READY", 0) != 0) return;

            // 发送文件内容
            uint64_t sent = 0;
            while (!ifs.eof()) {
                ifs.read(buf, sizeof(buf));
                std::streamsize cnt = ifs.gcount();
                ::send(sock_, buf, (int)cnt, 0);
                sent += cnt;
            }
            ifs.close();
            emit logMessage(QString("Uploaded %1 bytes from '%2'")
                                .arg(sent)
                                .arg(qlocal));
        }
        // ------------ QUIT ------------
        else if (op == "QUIT") {
            sendAll("QUIT\n");
            disconnectFromHost();
        }
        // ------------ 其它 ------------
        else {
            sendAll(line + "\n");
            char buf[256];
            int r = ::recv(sock_, buf, sizeof(buf), 0);
            QString resp = QString::fromLocal8Bit(buf, r).trimmed();
            emit logMessage(resp);
        }
    }).detach();
}
