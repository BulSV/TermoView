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
#ifdef DEBUG
        qDebug() << "[void OnePacket::readData()] ||| ba.size():" << itsReadData.size();
#endif

    if(itsPort->bytesAvailable() > itsPacketLenght - 1)
    {
        itsReadData.append(itsPort->readAll());
#ifdef DEBUG
        qDebug() << "ba =" << itsReadData.toHex();
        qDebug() << "ba.size():" << itsReadData.size();
        qDebug() << "ba.at(0):" << itsReadData.at(0);
        qDebug() << "ba.at(" << itsPacketLenght - 1 << "):" << itsReadData.at(itsPacketLenght - 1);
#endif

        if(itsReadData.at(0) == static_cast<char>(itsStartByte)
                && itsReadData.at(itsPacketLenght - 1) == static_cast<char>(itsStopByte))
        {
#ifdef DEBUG
            qDebug() << "emit ReadedData(ba):" << itsReadData.toHex();
#endif
            emit DataIsReaded(true);
            emit ReadedData(itsReadData);

        } else {
            emit DataIsReaded(false);
        }
    }

    itsReadData.clear();
}

QByteArray OnePacket::getReadData() const
{
    return itsReadData;
}
