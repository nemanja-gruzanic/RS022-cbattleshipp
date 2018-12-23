#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::writeLog(const QString & logMsg)
{
   ui->log->append(logMsg);
}

void MainWindow::writeIp(const QString & ipAddress)
{
    ui->serverIp->setText(ipAddress);
}