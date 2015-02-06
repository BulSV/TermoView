#ifndef DIALOG_H
#define DIALOG_H

#ifdef DEBUG
#include <QDebug>
#endif

#include <QDialog>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QGroupBox>
#include <QtSerialPort/QSerialPort>
#include <QByteArray>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QLCDNumber>
#include "ComPort.h"
#include "IProtocol.h"
#include "ReadSensorProtocol.h"

class Dialog : public QDialog
{
    Q_OBJECT

    QLabel *lPort;
    QComboBox *cbPort;
    QLabel *lBaud;
    QComboBox *cbBaud;
    QPushButton *bPortStart;
    QPushButton *bPortStop;
    QLabel *lRx;

    QLCDNumber *lcdCPUTermo;
    QLCDNumber *lcdSensor1Termo;
    QLCDNumber *lcdSensor2Termo;

    QGroupBox *gbCPU;
    QGroupBox *gbSensor1;
    QGroupBox *gbSensor2;

    QSerialPort *itsPort;
    ComPort *itsComPortReadSensors;
    IProtocol *itsSensorProtocol;

    QStringList itsTempSensorsList;

    // цвет индикации температуры >0 & <=0
    void setColorLCD(QLCDNumber *lcd, bool isHeat);
    // добавляет завершающие нули
    QString &addTrailingZeros(QString &str, int prec);

    QSystemTrayIcon *itsTray;
    QTimer *itsBlinkTimeNone;
    QTimer *itsBlinkTimeColor;
    QTimer *itsTimeToDisplay;

private slots:
    void openPort();
    void closePort();
    void cbPortChanged();
    void received(bool isReceived);
    // мигание надписи "Rx" при получении пакета
    void colorNoneRx();
    void colorIsRx();
    void display();
public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();
};

#endif // DIALOG_H
