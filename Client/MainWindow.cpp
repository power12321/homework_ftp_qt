#include "MainWindow.h"
#include <QtWidgets>
#include "WinsockClient.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    client_(new WinsockClient(this))
{
    auto *centr = new QWidget(this);
    setCentralWidget(centr);
    auto *vlay = new QVBoxLayout(centr);

    // IP+Port + Connect/Disconnect
    {
        auto *h = new QHBoxLayout;
        h->addWidget(new QLabel("IP:"));
        editIp_ = new QLineEdit("127.0.0.1"); h->addWidget(editIp_);
        h->addWidget(new QLabel("Port:"));
        editPort_ = new QLineEdit("2121"); editPort_->setFixedWidth(80);
        h->addWidget(editPort_);
        btnConnect_    = new QPushButton("Connect");
        btnDisconnect_ = new QPushButton("Disconnect");
        btnDisconnect_->setEnabled(false);
        h->addWidget(btnConnect_);
        h->addWidget(btnDisconnect_);
        vlay->addLayout(h);
    }

    // 命令 + Send
    {
        auto *h = new QHBoxLayout;
        editCmd_ = new QLineEdit; h->addWidget(editCmd_);
        btnSend_ = new QPushButton("Send");
        btnSend_->setEnabled(false);
        h->addWidget(btnSend_);
        vlay->addLayout(h);
    }

    // 日志区
    logArea_ = new QTextEdit; logArea_->setReadOnly(true);
    vlay->addWidget(logArea_);

    setWindowTitle("FTP Client");
    resize(600,400);

    // 信号槽
    connect(btnConnect_,    &QPushButton::clicked, this, &MainWindow::onConnect);
    connect(btnDisconnect_, &QPushButton::clicked, this, &MainWindow::onDisconnect);
    connect(btnSend_,       &QPushButton::clicked, this, &MainWindow::onSend);
    connect(client_, &WinsockClient::logMessage, this, &MainWindow::appendLog);
}

MainWindow::~MainWindow() {}

void MainWindow::onConnect()
{
    btnConnect_->setEnabled(false);
    btnDisconnect_->setEnabled(true);
    btnSend_->setEnabled(true);
    client_->connectToHost(editIp_->text(), editPort_->text().toUShort());
}

void MainWindow::onDisconnect()
{
    client_->disconnectFromHost();
    btnConnect_->setEnabled(true);
    btnDisconnect_->setEnabled(false);
    btnSend_->setEnabled(false);
}

void MainWindow::onSend()
{
    const QString cmd = editCmd_->text().trimmed();
    if (cmd.isEmpty()) return;
    client_->sendCommand(cmd);
    editCmd_->clear();
}

void MainWindow::appendLog(const QString &msg)
{
    logArea_->append(msg);
}
