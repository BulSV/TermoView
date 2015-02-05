#ifndef ONEPACKET_H
#define ONEPACKET_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QByteArray>

class ComPort: public QObject
{
    Q_OBJECT
public:
    enum COMMODE {
        READ = 0x01,
        WRITE = 0x02,
        READWRITE = READ | WRITE
    };
    ComPort(QSerialPort *port,
            ComPort::COMMODE comMode = READWRITE,
            int startByte = 0x55,
            int stopByte = 0xAA,
            int packetLenght = 8,
            QObject *parent = 0);
    QByteArray getReadData() const;
    void setWriteData(const QByteArray &data);
    QByteArray getWriteData() const;
public slots:
    void writeData();
signals:
    void DataIsReaded(bool);
    void ReadedData(QByteArray);
    void DataIsWrited(bool);
    void WritedData(QByteArray);
private slots:
    void readData();
private:
    QSerialPort *itsPort;

    COMMODE itsComMode;

    QByteArray itsReadData;
    QByteArray itsWriteData;

    int itsStartByte;
    int itsStopByte;
    int itsPacketLenght;
    int m_counter;
};

#endif // ONEPACKET_H
