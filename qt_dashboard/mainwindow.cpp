#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "udpreceiver.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    UdpReceiver* receiver = new UdpReceiver(this);
    connect(receiver, &UdpReceiver::telemetryReceived, this, &MainWindow::updateDashboard);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateDashboard(double time, double altitude, double velocity, double thrust, double qx, double qy, double qz, double qw)
{
    ui->lblAltitude->setText("Altitude: " + QString::number(altitude) + " m");
    ui->lblVelocity->setText("Velocity: " + QString::number(velocity) + " m/s");
    ui->lblThrust->setText("Thrust: " + QString::number(thrust) + " N");
}
