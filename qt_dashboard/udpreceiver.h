#ifndef UDPRECEIVER_H
#define UDPRECEIVER_H

#include <QObject>
#include <QUdpSocket>

class UdpReceiver : public QObject
{
    Q_OBJECT
public:
    explicit UdpReceiver(QObject *parent = nullptr);

signals:
    //part 1 signal
    //when the telemetry arrived and is parsed signal to MainWindow
    //so it can use the new data
    void telemetryReceived(double time, double alt, double vel, double thrust, double qx, double qy, double qz, double qw);

public slots:
    //part 2 slot
    //when os drops a new udp package into network card
    //this function will run and read the data automatically
    void processPendingDatagrams();

private:
    QUdpSocket* socket;
};

#endif // UDPRECEIVER_H
