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

    QLabel *lLogoPix;
    QLabel *lCPUTermoPix;
    QLabel *lSensor1TermoPix;
    QLabel *lSensor2TermoPix;

    QLabel *lCPUTermo;
    QLabel *lSensor1Termo;
    QLabel *lSensor2Termo;

    QGroupBox *gbCPU;
    QGroupBox *gbSensor1;
    QGroupBox *gbSensor2;

    QSerialPort *itsPort;
    OnePacket *itsOnePacket;

    QByteArray toWord(int nInt);
    void swapBytes(QByteArray &ba);
    QString toHumanHex(QByteArray ba);
    QString mSecToSec(int time);
    void clearInfo();

    QSystemTrayIcon *itsTray;

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

signals:

public slots:
    void openPort();
    void cbPortChanged();
    void answer();
};

#endif // DIALOG_H
