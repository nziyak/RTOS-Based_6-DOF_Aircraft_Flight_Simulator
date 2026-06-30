#include "udpreceiver.h"

#include <QNetworkDatagram>
#include <QStringList>

UdpReceiver::UdpReceiver(QObject *parent) : QObject{parent}
{
    socket = new QUdpSocket(this);

    //simulator sends the packages to localhost:5005, listen that port
    socket->bind(QHostAddress::LocalHost, 5005);

    //Qt's connect property.
    //if readyRead signal comes from the socket object,
    //it runs the processPendingDatagrams function
    connect(socket, &QUdpSocket::readyRead, this, &UdpReceiver::processPendingDatagrams);
}

void UdpReceiver::processPendingDatagrams()
{
    while(socket->hasPendingDatagrams())
    {
        QNetworkDatagram datagram = socket->receiveDatagram(); //returns a raw byte array
        QByteArray byteArr = datagram.data();
        QString text = QString::fromUtf8(byteArr); //convert that byte array into a string
        QStringList datalist = text.split(","); //split that string with commas

        //take the values
        double time = datalist[0].toDouble();
        double alt = datalist[1].toDouble();
        double vel = datalist[2].toDouble();
        double thrust = datalist[3].toDouble();
        double orx = datalist[4].toDouble();
        double ory = datalist[5].toDouble();
        double orz = datalist[6].toDouble();
        double orw = datalist[7].toDouble();

        //send the values to MainWindow
        emit telemetryReceived(time, alt, vel, thrust, orx, ory, orz, orw);
    }
}
