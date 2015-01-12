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
    QLabel *lPort;
    QComboBox *cbPort;
    QPushButton *bPortOpen;  

    QLabel *lCPUTermo;
    QLabel *lSensor1Termo;
    QLabel *lSensor2Termo;

    QGroupBox *gbCPU;
    QGroupBox *gbSensor1;
    QGroupBox *gbSensor2;

    QSerialPort *itsPort;
    OnePacket *itsOnePacket;

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
