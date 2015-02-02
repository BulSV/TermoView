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
#include "OnePacket.h"

class Dialog : public QDialog
{
    Q_OBJECT
    enum SENSORS {
        CPU, SENSOR1, SENSOR2
    };

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
    OnePacket *itsOnePacket;

    float itsPrevCPUTemp;
    float itsPrevSensor1Temp;
    float itsPrevSensor2Temp;

    QStringList itsTempSensorsList;

    // преобразует word в byte
    int wordToInt(QByteArray ba);
    // преобразование byte в word
    QByteArray toWord(int nInt);
    // меняет байты местами
    void swapBytes(QByteArray &ba);
    // Преобразует hex вида 000000 в 00 00 00
    QString toHumanHex(QByteArray ba);
    // Преобразует милисекунды в секунды
    QString mSecToSec(int time);
    // определяет температуру
    float tempSensors(int temp);
    // определяет температуру кристалла
    float tempCPU(int temp);
    // коррекция скачков температуры
    float tempCorr(float temp, SENSORS sensor);
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
    void received(QByteArray ba);
    // мигание надписи "Rx" при получении пакета
    void colorNoneRx();
    void colorIsRx();
    void display();

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();
};

#endif // DIALOG_H
