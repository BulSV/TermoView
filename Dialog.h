#ifndef DIALOG_H
#define DIALOG_H

#ifdef DEBUG
#include <QDebug>
#endif

#include <QDialog>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QSpinBox>
#include <QGroupBox>
#include <QtSerialPort/qserialport.h>
#include <QSettings>
#include <QMultiMap>
#include <QByteArray>
#include <QSound>
#include <QSystemTrayIcon>
#include "OnePacket.h"

class Dialog : public QDialog
{
    Q_OBJECT
    QLabel *lPort;
    QComboBox *cbPort;
    QPushButton *bPortOpen;        

    QLabel *lFreq;
    QSpinBox *sbFreq;
    QPushButton *bFreqSet;

    QPushButton *bSetInitialPosition;

    QGroupBox *gbInfo;

    QLabel *lSM1;
    QLabel *lSM2;
    QLabel *lSM3;
    QLabel *lSM4;
    QLabel *lSM5;

    QLabel *lCurrentFreq;

    QLabel *lLogo;

    QSerialPort *itsPort;
    OnePacket *itsOnePacket;   

    QString itsConfigFileName;
    QSettings configSettings; // для файла config.ini

    QMultiMap <int, int> freqStepsMap;

    QByteArray sendData;    

    QString itsErrorFileName;
    QSound itsSoundError;

    bool itsIsSetInit;

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
    void readSettings();
    void writeConfigSettings();    
    void setFreqFVP();
    void InitialPosition();
    void send(int mode, int device, int freq);
    void cbPortChanged();
    void answer();
    void recalledAnswer(int recalled, int statistic);
};

#endif // DIALOG_H
