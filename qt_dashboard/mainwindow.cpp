#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "udpreceiver.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //create the line(seri) object
    altitudeSeries = new QLineSeries();
    altitudeSeries->setName("Altitude (m)");

    //create the chart object and add the line in it
    altitudeChart = new QChart();
    altitudeChart->addSeries(altitudeSeries);
    altitudeChart->createDefaultAxes(); //create the x and y axes automatically
    ui->chartView->setChart(altitudeChart); //let ui draw that chart

    UdpReceiver* receiver = new UdpReceiver(this);
    connect(receiver, &UdpReceiver::telemetryReceived, this, &MainWindow::updateDashboard);

    //change the theme
    this->setStyleSheet(
        "QMainWindow { background-color: #1e1e24; }"
        "QLabel { color: #00ffcc; font-size: 14px; font-weight: bold; font-family: 'Consolas', monospace; }"
        "QChartView { background-color: #1e1e24; border: 1px solid #333340; }"
    );

    // Grafiğin arka planını da karanlık temaya uyduralım
    altitudeChart->setBackgroundBrush(QBrush(QColor("#1e1e24")));
    altitudeChart->setTitleBrush(QBrush(QColor("#00ffcc")));
    altitudeChart->legend()->setLabelColor(QColor("#ffffff"));

    // Eksenlerin (X ve Y oklarının) renklerini beyaz/gri yapalım
    altitudeChart->axes(Qt::Horizontal).first()->setLabelsColor(QColor("#cccccc"));
    altitudeChart->axes(Qt::Vertical).first()->setLabelsColor(QColor("#cccccc"));
    altitudeChart->axes(Qt::Horizontal).first()->setGridLineColor(QColor("#333340"));
    altitudeChart->axes(Qt::Vertical).first()->setGridLineColor(QColor("#333340"));

    // Çizgiyi (Series) fosforlu cyan (turkuaz) rengi ve kalın yapalım
    QPen pen(QColor("#00ffcc"));
    pen.setWidth(3);
    altitudeSeries->setPen(pen);

    altitudeChart->axes(Qt::Horizontal).first()->setTitleText("Time (s)");
    altitudeChart->axes(Qt::Vertical).first()->setTitleText("Altitude (m)");

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

    altitudeSeries->append(time,altitude); //add the new time and altitude data to the line

    //move the axis so data is visible at every moment

    //x axis (time)
    if(time < 15.0) //if we are in the first 15 seconds show between 0-15 seconds
    {
        altitudeChart->axes(Qt::Horizontal).first()->setRange(0,15);
    }
    else
    {
        altitudeChart->axes(Qt::Horizontal).first()->setRange(time - 15.0, time);
    }

    //y axis (altitude)
    //adjust according to the altitude of the aircraft
    //100 meters below and 500 meters above will be shown
    altitudeChart->axes(Qt::Vertical).first()->setRange(0, altitude + 500);
}
