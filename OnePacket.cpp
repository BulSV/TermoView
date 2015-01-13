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
    QByteArray buffer;

    if(itsPort->bytesAvailable() > itsPacketLenght - 1)
    {
#ifdef DEBUG
        if(!buffer.isEmpty()) {
            qDebug() << "buffer =" << buffer.toHex();
        } else {
            qDebug() << "buffer is empty!";
        }
#endif
        ba.append(buffer);
        ba.append(itsPort->readAll());
        buffer.clear();
#ifdef DEBUG
        qDebug() << "ba =" << ba.toHex();
#endif
#ifdef DEBUG
        qDebug() << "ba.size():" << ba.size();
        qDebug() << "ba.at(0):" << ba.at(0);
//        qDebug() << "ba.at(" << itsPacketLenght - 1 << "):" << ba.at(itsPacketLenght - 1);
#endif
        while(ba.size() >= itsPacketLenght) {
            while(ba.at(0) != static_cast<char>(itsStartByte)) {
                ba.remove(0, 1);
            }

            if(ba.size() < itsPacketLenght) {
                buffer.append(ba);
                break;
            }

            if(ba.at(0) == static_cast<char>(itsStartByte)
                    && ba.at(itsPacketLenght - 1) == static_cast<char>(itsStopByte))
            {
#ifdef DEBUG
                qDebug() << "emit ReadedData(ba):" << ba.toHex();
#endif
                emit ReadedData(ba);

                ba.remove(0, itsPacketLenght);
#ifdef DEBUG
        qDebug() << "ba.remove(0, itsPacketLenght) =" << ba.toHex();
#endif

                if(ba.size() > 0) {
                    buffer.append(ba);
                }
            } else {
                ba.remove(0, 1);
#ifdef DEBUG
        qDebug() << "ba.remove(0, 1) =" << ba.toHex();
#endif
            }
        }
    }
}

QByteArray OnePacket::getReadData() const
{
    return itsReadData;
}
