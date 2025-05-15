#include "MainWindow.h"
#include <QtWidgets>
#include "WinsockServer.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    server_(new WinsockServer(this))
{
    // --- UI 构建 ---
    auto *central = new QWidget(this);
    setCentralWidget(central);
    auto *vlay = new QVBoxLayout(central);

    auto *hlay = new QHBoxLayout;
    hlay->addWidget(new QLabel("Port:"));
    editPort_ = new QLineEdit("2121");
    editPort_->setFixedWidth(80);
    hlay->addWidget(editPort_);
    btnStart_ = new QPushButton("Start");
    btnStop_  = new QPushButton("Stop");
    btnStop_->setEnabled(false);
    hlay->addWidget(btnStart_);
    hlay->addWidget(btnStop_);
    vlay->addLayout(hlay);

    logArea_ = new QTextEdit;
    logArea_->setReadOnly(true);
    vlay->addWidget(logArea_);

    setWindowTitle("FTP Server");
    resize(600, 400);

    // --- 信号槽绑定 ---
    connect(btnStart_, &QPushButton::clicked, this, &MainWindow::onStartServer);
    connect(btnStop_,  &QPushButton::clicked, this, &MainWindow::onStopServer);
    connect(server_,   &WinsockServer::logMessage, this, &MainWindow::appendLog);
}

MainWindow::~MainWindow() {}

void MainWindow::onStartServer()
{
    quint16 port = editPort_->text().toUShort();
    server_->start(port);
    btnStart_->setEnabled(false);
    btnStop_->setEnabled(true);
}

void MainWindow::onStopServer()
{
    server_->stop();
    btnStart_->setEnabled(true);
    btnStop_->setEnabled(false);
}

void MainWindow::appendLog(const QString &msg)
{
    logArea_->append(msg);
}
