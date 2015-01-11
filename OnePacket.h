#ifndef ONEPACKET_H
#define ONEPACKET_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QByteArray>
#include <QTimer>
#include <QMultiMap>

class OnePacket: public QObject
{
    Q_OBJECT
    QSerialPort *itsPort;
    QByteArray itsSendData;
    QByteArray itsReadData;
    int itsNumRecalls;
    int itsPacketLenght;
    int itsBytesDelay;
    int itsReadDelay;
    int itsReadErrorDelay;
    int itsNumResends;
    QTimer *itsByteSendTimer;
    QTimer *itsTimeStartRead;
    QTimer *itsTimeReadDataProcessing;
    int itsCurrentByte;
    int itsCurrentRecall;
    int itsCurrentResend;
private slots:
    void sendData();
    void readData();
    void wasError();
    void stopStartReadTimer();
public slots:
    void sendData(QByteArray toSend);
    QByteArray getReadData() const;
    void setToActive(bool isActive);
signals:
    void DataIsReaded(bool);
    void DataReadTimeout();
    void DataIsPreperedToSend(bool);
    void NextByte(int);
    void ReadedData(QByteArray);
public:
    OnePacket(QSerialPort *port,
              int numRecalls = 1,
              int packetLenght = 8,
              int bytesDelay = 1,
              int readDelay = 1,
              int readErrorDelay = 1000,
              int numResends = 0,
              QObject *parent = 0);
    int valueNumRecalls() const;
};

#endif // ONEPACKET_H
