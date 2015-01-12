#ifdef DEBUG
#include <QDebug>
#endif

#include "OnePacket.h"
#include <QApplication>
#include <QTime>

OnePacket::OnePacket(QSerialPort *port,
                     int startByte,
                     int stopByte,
                     int packetLenght,
                     QObject *parent)
    : QObject(parent),
      itsPort(port),
      itsStartByte(startByte),
      itsStopByte(stopByte),
      itsPacketLenght(packetLenght)
{
    itsReadData.clear();

    connect(itsPort, SIGNAL(readyRead()), this, SLOT(readData()));
}

void OnePacket::readData()
{
    QByteArray ba;

    if(itsPort->bytesAvailable() > itsPacketLenght - 1)
    {
        ba.append(itsPort->readAll());
#ifdef DEBUG
        qDebug() << "ba =" << ba.toHex();
#endif
#ifdef DEBUG
        qDebug() << "ba.size():" << ba.size();
        qDebug() << "ba.at(0):" << ba.at(0);
        qDebug() << "ba.at(" << itsPacketLenght - 1 << "):" << ba.at(itsPacketLenght - 1);
#endif
        if(ba.size() >= itsPacketLenght
                && ba.at(0) == static_cast<char>(itsStartByte)
                && ba.at(itsPacketLenght - 1) == static_cast<char>(itsStopByte))
        {
#ifdef DEBUG
            qDebug() << "emit ReadedData(ba):" << ba.toHex();
#endif
            emit ReadedData(ba);
        }
    }
}

QByteArray OnePacket::getReadData() const
{
    return itsReadData;
}
