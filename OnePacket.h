#ifndef ONEPACKET_H
#define ONEPACKET_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QByteArray>

class OnePacket: public QObject
{
    Q_OBJECT
    QSerialPort *itsPort;

    QByteArray itsReadData;

    int itsStartByte;
    int itsStopByte;
    int itsPacketLenght;
private slots:
    void readData();
public slots:
    QByteArray getReadData() const;
signals:
    void DataIsReaded(bool);
    void ReadedData(QByteArray);
public:
    OnePacket(QSerialPort *port,
              int startByte = 0x55,
              int stopByte = 0xAA,
              int packetLenght = 8,
              QObject *parent = 0);
};

#endif // ONEPACKET_H
