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
    QPushButton *bPortOpen;

    QLabel *lCPUTermo;
    QLabel *lSensor1Termo;
    QLabel *lSensor2Termo;

    QGroupBox *gbCPU;
    QGroupBox *gbSensor1;
    QGroupBox *gbSensor2;

    QSerialPort *itsPort;
    OnePacket *itsOnePacket;

    float itsPrevCPUTemp;
    float itsPrevSensor1Temp;
    float itsPrevSensor2Temp;

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
    float temperature(int temp);
    // определяет температуру кристалла
    float tempCPU(int temp);
    // коррекция скачков температуры
    float tempCorr(float temp, SENSORS sensor);

    QSystemTrayIcon *itsTray;

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

signals:

public slots:
    void openPort();
    void cbPortChanged();
    void answer(QByteArray ba);
};

#endif // DIALOG_H
