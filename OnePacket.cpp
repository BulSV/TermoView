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
      itsPacketLenght(packetLenght),
      m_counter(0)
{
    itsReadData.clear();
    itsPort->setReadBufferSize(1); // for reading 1 byte at a time

    connect(itsPort, SIGNAL(readyRead()), this, SLOT(readData()));
}

void OnePacket::readData()
{
#ifdef DEBUG
        qDebug() << "[void OnePacket::readData()] ||| itsPort->bytesAvailable():" << itsPort->bytesAvailable();
        qDebug() << "[void OnePacket::readData()] ||| ba.size():" << itsReadData.size();
#endif
    QByteArray buffer;

    if(itsPort->bytesAvailable() > 0) {
        buffer.append(itsPort->read(1));

#ifdef DEBUG
        qDebug() << "buffer =" << buffer.toHex();
        qDebug() << "ba =" << itsReadData.toHex();
        qDebug() << "ba.size():" << itsReadData.size();
#endif
        if(!m_counter && buffer.at(0) == static_cast<char>(itsStartByte)) {
#ifdef DEBUG
            qDebug() << "BEGIN";
#endif
            itsReadData.append(buffer);
            ++m_counter;
        } else if(m_counter && m_counter < itsPacketLenght) {
#ifdef DEBUG
            qDebug() << "CONTINUE";
#endif
            itsReadData.append(buffer);
            ++m_counter;

            if((m_counter == itsPacketLenght)
                    && itsReadData.at(itsPacketLenght - 1) == static_cast<char>(itsStopByte)) {
#ifdef DEBUG
                qDebug() << "emit ReadedData(ba):" << itsReadData.toHex();
                qDebug() << "RECEIVED";
#endif
                emit DataIsReaded(true);
                emit ReadedData(itsReadData);

                itsReadData.clear();
                m_counter = 0;
            }
        } else {
            emit DataIsReaded(false);

            itsReadData.clear();
            m_counter = 0;
        }
    }
}

QByteArray OnePacket::getReadData() const
{
    return itsReadData;
}
