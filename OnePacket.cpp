#ifdef DEBUG
#include <QDebug>
#endif

#include "OnePacket.h"
#include <QApplication>
#include <QTime>

OnePacket::OnePacket(QSerialPort *port,
                     int numRecalls,
                     int packetLenght,
                     int bytesDelay,
                     int readDelay,
                     int readErrorDelay,
                     int numResends,
                     QObject *parent)
    : QObject(parent),
      itsPort(port),
      itsNumRecalls(numRecalls),
      itsPacketLenght(packetLenght),
      itsBytesDelay(bytesDelay),
      itsReadDelay(readDelay),
      itsReadErrorDelay(readErrorDelay),
      itsNumResends(numResends),
      itsByteSendTimer(new QTimer(this)),
      itsTimeStartRead(new QTimer(this)),
      itsTimeReadDataProcessing(new QTimer(this)),
      itsCurrentByte(0),
      itsCurrentRecall(0),
      itsCurrentResend(0)
{
    itsReadData.clear();
    itsByteSendTimer->setInterval(itsBytesDelay);
    itsTimeStartRead->setInterval(itsReadDelay);
    itsTimeReadDataProcessing->setInterval(itsReadErrorDelay);

    connect(itsPort, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(this, SIGNAL(DataIsPreperedToSend(bool)), SLOT(sendData()));
    connect(this, SIGNAL(NextByte(int)), itsByteSendTimer, SLOT(start()));
    connect(itsByteSendTimer, SIGNAL(timeout()), this, SLOT(sendData()));
    connect(itsTimeStartRead, SIGNAL(timeout()), this, SLOT(stopStartReadTimer()));
    connect(itsTimeReadDataProcessing, SIGNAL(timeout()), this, SLOT(wasError()));
}

int OnePacket::valueNumRecalls() const
{
    return itsNumRecalls;
}

void OnePacket::sendData()
{
    itsPort->reset();
    QByteArray ba;

    if(itsCurrentByte == itsPacketLenght)
    {
        itsCurrentByte = 0;
        itsByteSendTimer->stop();       
        itsTimeReadDataProcessing->start();
        itsTimeStartRead->start();
#ifdef DEBUG
        qDebug() << "void OnePacket::sendData(): itsSendData =" << itsSendData.toHex() << QTime::currentTime() << "ms" << QTime::currentTime().msec();
#endif
    }
    else
    {
        ba[0] = itsSendData[itsCurrentByte];
        itsPort->write(ba);
#ifdef DEBUG
        qDebug() << "void OnePacket::sendData(): ba =" << ba.toHex() << QTime::currentTime() << "ms" << QTime::currentTime().msec();
#endif
        emit NextByte(++itsCurrentByte);
    }
}

void OnePacket::readData()
{    
    if(itsPort->bytesAvailable() > 0 && itsTimeReadDataProcessing->isActive())
    {        
        QByteArray ba;
        while(itsTimeStartRead->isActive())
        {
            qApp->processEvents();

            if(itsPort->bytesAvailable() > 0)
            {
                ba.append(itsPort->readAll());                
#ifdef DEBUG
            qDebug() << "ba =" << ba.toHex();
            qDebug() << "itsTimeStartRead->isActive():" << itsTimeStartRead->isActive();
#endif
            }            
        }
#ifdef DEBUG
        qDebug() << "\n\nvoid OnePacket::readData(): itsPort->readAll() =" << ba.toHex() << QTime::currentTime() << "ms" << QTime::currentTime().msec();
#endif
        int index = 0;
        int packet = 0;
        while(itsTimeReadDataProcessing->isActive() || packet != itsNumRecalls)
        {
            if(index + itsPacketLenght > ba.size())
            {
#ifdef DEBUG
                qDebug() << "break" << ba.size();
#endif
                break;
            }

            if((ba.at(index) == itsSendData.at(0)) && (ba.at(itsPacketLenght - 1 + index) == itsSendData.at(itsPacketLenght - 1)))
            {
                index += itsPacketLenght;
                ++packet;                
#ifdef DEBUG
                qDebug() << "void OnePacket::readData(): index =" << index;
                qDebug() << "void OnePacket::readData(): packet =" << packet;
                qDebug() << "void OnePacket::readData(): ba.at(" << itsPacketLenght - 1 + index << "): ba = " << ba.toHex() << "& itsSendData =" << itsSendData.toHex();
#endif
            }
            else
            {
                ba.remove(index, 1);
#ifdef DEBUG
        qDebug() << "void OnePacket::readData(): ba.remove(" << index << ", 1) =" << ba.toHex();
#endif
            }
        }

        for(int indexPacket = 0; indexPacket < itsNumRecalls; ++indexPacket)
        {
            itsReadData = ba;
            itsReadData.truncate(itsPacketLenght);
            ba.remove(0, itsPacketLenght);

            if((itsReadData.size() == itsPacketLenght)
                    && (itsReadData.at(0) == itsSendData.at(0))
                    && (itsReadData.at(itsPacketLenght - 1) == itsSendData.at(itsPacketLenght - 1)))
            {
#ifdef DEBUG
                qDebug() << "void OnePacket::readData(): ba =" << itsReadData.toHex() << QTime::currentTime() << "ms" << QTime::currentTime().msec();
#endif
                ++itsCurrentRecall;
#ifdef DEBUG
                qDebug() << "void OnePacket::readData(): itsCurrentRecall =" << itsCurrentRecall;
#endif
                if(itsCurrentRecall == itsNumRecalls || itsNumRecalls == 0)  // itsNumRecalls == 0 для случая, когда не нужно читать ответ
                {
                    itsTimeReadDataProcessing->stop();
                    itsCurrentRecall = 0;
                }

                itsCurrentResend = 0;
                emit DataIsReaded(true);
            }
            else // когда ответ не совпадает с ожидаемым
            {
                if(itsCurrentResend < itsNumResends)
                {
                    ++itsCurrentResend;
                    sendData();
                }
                else
                {
                    itsCurrentResend = 0;
                    itsCurrentRecall = 0;
                    itsTimeReadDataProcessing->stop();

                    itsReadData.resize(itsPacketLenght);
                    itsReadData.fill(-1, itsPacketLenght);

#ifdef DEBUG
                    qDebug() << "void OnePacket::readData(): itsReadData error =" << itsReadData.toHex(); // 08.11.2012
#endif
                    emit DataIsReaded(false);
                }
            }
        }
    }    
}

// когда нет никакого ответа
void OnePacket::wasError()
{
    if(itsCurrentResend < itsNumResends)
    {
        ++itsCurrentResend;
        sendData();
    }
    else
    {
        itsCurrentResend = 0;
        itsCurrentRecall = 0;
        itsTimeReadDataProcessing->stop();

        itsReadData.resize(itsPacketLenght);
        itsReadData.fill(0, itsPacketLenght);

#ifdef DEBUG
        qDebug() << "void OnePacket::wasError(): itsReadData.fill =" << itsReadData.toHex(); // 08.11.2012
#endif
        emit DataReadTimeout();
    }
}

void OnePacket::stopStartReadTimer()
{
    itsTimeStartRead->stop();
}

void OnePacket::sendData(QByteArray toSend)
{
    itsSendData.clear();
    itsSendData = toSend;
    itsReadData.clear();
    emit DataIsPreperedToSend(true);
}

QByteArray OnePacket::getReadData() const
{    
    return itsReadData;
}

void OnePacket::setToActive(bool isActive)
{
    if(isActive)
    {
        connect(itsPort, SIGNAL(readyRead()), this, SLOT(readData()));
        itsPort->reset();
    }
    else
    {
        itsPort->reset();
        disconnect(itsPort, SIGNAL(readyRead()), this, SLOT(readData()));
    }
}
