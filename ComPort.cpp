#ifdef DEBUG
#include <QDebug>
#endif

#include "ComPort.h"
#include <QApplication>
#include <QTime>

ComPort::ComPort(QSerialPort *port,
                 ComPort::COMMODE comMode,
                 int startByte,
                 int stopByte,
                 int packetLenght,
                 QObject *parent)
    : QObject(parent),
      itsPort(port),
      itsComMode(comMode),
      itsStartByte(startByte),
      itsStopByte(stopByte),
      itsPacketLenght(packetLenght),
      m_counter(0)
{
    itsReadData.clear();
    itsPort->setReadBufferSize(1); // for reading 1 byte at a time

    connect(itsPort, SIGNAL(readyRead()), this, SLOT(readData()));
}

void ComPort::readData()
{
    if(itsComMode != WRITE) {
        QByteArray buffer;

        if(itsPort->bytesAvailable() > 0) {
            buffer.append(itsPort->read(1));

            if(!m_counter && buffer.at(0) == static_cast<char>(itsStartByte)) {
                itsReadData.append(buffer);
                ++m_counter;
            } else if(m_counter && m_counter < itsPacketLenght) {
                itsReadData.append(buffer);
                ++m_counter;

                if((m_counter == itsPacketLenght)
                        && itsReadData.at(itsPacketLenght - 1) == static_cast<char>(itsStopByte)) {
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
}

QByteArray ComPort::getReadData() const
{
    return itsReadData;
}

void ComPort::setWriteData(const QByteArray &data)
{
    itsWriteData = data;
}

QByteArray ComPort::getWriteData() const
{
    return itsWriteData;
}

void ComPort::writeData()
{
    if(itsComMode != READ) {
        itsPort->write(itsWriteData);
        emit DataIsWrited(true);
        emit WritedData(itsWriteData);
    }
}
