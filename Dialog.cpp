#include "Dialog.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QApplication>
#include <QFile>
#include <QDesktopWidget>
#include <QShortcut>
#include <QSerialPortInfo>
#include <QPalette>

#define STARTBYTE 0x55
#define STOPBYTE 0xAA
#define BYTESLENTH 8

#define NEGATIVE 32768 // 2^15
#define OFFSET 65536 // 2^16
#define SLOPE 128

#define CPU_FACTOR 0.537
#define CPU_OFFSET 900
#define CPU_SLOPE 2.95

#define ACCURACY 0.02

#define FORMAT 'f'
#define PRECISION 2

#define BLINKTIME 500 // ms
#define DISPLAYTIME 100 // ms

Dialog::Dialog(QWidget *parent) :
        QDialog(parent),
        lPort(new QLabel(QString::fromUtf8("Port"), this)),
        cbPort(new QComboBox(this)),
        lBaud(new QLabel(QString::fromUtf8("Baud"), this)),
        cbBaud(new QComboBox(this)),
        bPortStart(new QPushButton(QString::fromUtf8("Start"), this)),
        bPortStop(new QPushButton(QString::fromUtf8("Stop"), this)),
        lRx(new QLabel("          Rx          ", this)),
        lcdCPUTermo(new QLCDNumber(this)),
        lcdSensor1Termo(new QLCDNumber(this)),
        lcdSensor2Termo(new QLCDNumber(this)),
        gbCPU(new QGroupBox(QString::fromUtf8("CPU"), this)),
        gbSensor1(new QGroupBox(QString::fromUtf8("Sensor 1"), this)),
        gbSensor2(new QGroupBox(QString::fromUtf8("Sensor 2"), this)),
        itsPort(new QSerialPort(this)),
        itsComPortReadSensors(new ComPort(itsPort, ComPort::READ, STARTBYTE, STOPBYTE, BYTESLENTH, this)),
        itsSensorProtocol(new ReadSensorProtocol(itsComPortReadSensors, this)),
        itsTray (new QSystemTrayIcon(QPixmap(":/Termo.png"), this)),
        itsBlinkTimeNone(new QTimer(this)),
        itsBlinkTimeColor(new QTimer(this)),
        itsTimeToDisplay(new QTimer(this))
{
    setLayout(new QVBoxLayout(this));

    lRx->setStyleSheet("background: red; font: bold; font-size: 10pt");
    lRx->setFrameStyle(QFrame::Box);
    lRx->setAlignment(Qt::AlignCenter);
    lRx->setMargin(2);

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(lPort, 0, 0);
    grid->addWidget(cbPort, 0, 1);
    grid->addWidget(lBaud, 1, 0);
    grid->addWidget(cbBaud, 1, 1);
    // помещаю логотип фирмы
    grid->addWidget(new QLabel("<img src=':/elisat.png' height='40' width='150'/>", this), 0, 2, 2, 4, Qt::AlignRight);
    grid->addWidget(bPortStart, 2, 1);
    grid->addWidget(bPortStop, 2, 2);
    grid->addWidget(lRx, 2, 2, 1, 4, Qt::AlignRight);
    grid->setSpacing(5);

    QVBoxLayout *verCPU = new QVBoxLayout;
    verCPU->addWidget(new QLabel("<img src=':/Termo.png' height='50' width='50'/>", this), 0, Qt::AlignCenter);
    verCPU->addWidget(lcdCPUTermo, 0, Qt::AlignCenter);
    verCPU->setSpacing(5);

    QVBoxLayout *verSensor1 = new QVBoxLayout;
    verSensor1->addWidget(new QLabel("<img src=':/Termo.png' height='50' width='50'/>", this), 0, Qt::AlignCenter);
    verSensor1->addWidget(lcdSensor1Termo, 0, Qt::AlignCenter);
    verSensor1->setSpacing(5);

    QVBoxLayout *verSensor2 = new QVBoxLayout;
    verSensor2->addWidget(new QLabel("<img src=':/Termo.png' height='50' width='50'/>", this), 0, Qt::AlignCenter);
    verSensor2->addWidget(lcdSensor2Termo, 0, Qt::AlignCenter);
    verSensor2->setSpacing(5);

    gbCPU->setLayout(verCPU);
    gbSensor1->setLayout(verSensor1);
    gbSensor2->setLayout(verSensor2);

    QHBoxLayout *hor = new QHBoxLayout;
    hor->addWidget(gbCPU, 0, Qt::AlignCenter);
    hor->addWidget(gbSensor1, 0, Qt::AlignCenter);
    hor->addWidget(gbSensor2, 0, Qt::AlignCenter);
    hor->setSpacing(5);

    layout()->addItem(grid);
    layout()->addItem(hor);
    layout()->setSpacing(5);

    // делает окно фиксированного размера
    this->layout()->setSizeConstraint(QLayout::SetFixedSize);

    QStringList portsNames;

    foreach(QSerialPortInfo portsAvailable, QSerialPortInfo::availablePorts())
    {
        portsNames << portsAvailable.portName();
    }

    cbPort->addItems(portsNames);
#if defined (Q_OS_LINUX)
    cbPort->setEditable(true); // TODO Make correct viewing available ports in Linux
#else
    cbPort->setEditable(false);
#endif

    QStringList portsBauds;
    portsBauds << "115200" << "57600" << "38400";
    cbBaud->addItems(portsBauds);
    cbBaud->setEditable(false);
    bPortStop->setEnabled(false);

    itsTray->setVisible(true);
    itsBlinkTimeNone->setInterval(BLINKTIME);
    itsBlinkTimeColor->setInterval(BLINKTIME);
    itsTimeToDisplay->setInterval(DISPLAYTIME);

    QList<QLCDNumber*> list;
    list << lcdCPUTermo << lcdSensor1Termo << lcdSensor2Termo;
    foreach(QLCDNumber *lcd, list) {
        lcd->setMinimumSize(80, 40);
        lcd->setDigitCount(6);
        lcd->setSegmentStyle(QLCDNumber::Flat);
        lcd->setFrameStyle(QFrame::NoFrame);
    }

    connect(bPortStart, SIGNAL(clicked()), this, SLOT(openPort()));
    connect(bPortStop, SIGNAL(clicked()), this, SLOT(closePort()));
    connect(cbPort, SIGNAL(currentIndexChanged(int)), this, SLOT(cbPortChanged()));
    connect(cbBaud, SIGNAL(currentIndexChanged(int)), this, SLOT(cbPortChanged()));
    connect(itsSensorProtocol, SIGNAL(DataIsReaded(bool)), this, SLOT(received(bool)));
    connect(itsBlinkTimeColor, SIGNAL(timeout()), this, SLOT(colorIsRx()));
    connect(itsBlinkTimeNone, SIGNAL(timeout()), this, SLOT(colorNoneRx()));
    connect(itsTimeToDisplay, SIGNAL(timeout()), this, SLOT(display()));

    QShortcut *aboutShortcut = new QShortcut(QKeySequence("F1"), this);
    connect(aboutShortcut, SIGNAL(activated()), qApp, SLOT(aboutQt()));
}

Dialog::~Dialog()
{
    itsTray->setVisible(false);
    itsPort->close();
}

void Dialog::openPort()
{
    itsPort->close();
    itsPort->setPortName(cbPort->currentText());

    if(itsPort->open(QSerialPort::ReadOnly))
    {
        switch (cbBaud->currentIndex()) {
        case 0:
            itsPort->setBaudRate(QSerialPort::Baud115200);
            break;
        case 1:
            itsPort->setBaudRate(QSerialPort::Baud57600);
            break;
        case 2:
            itsPort->setBaudRate(QSerialPort::Baud38400);
            break;
        default:
            itsPort->setBaudRate(QSerialPort::Baud115200);
            break;
        }

        itsPort->setDataBits(QSerialPort::Data8);
        itsPort->setParity(QSerialPort::NoParity);
        itsPort->setFlowControl(QSerialPort::NoFlowControl);

        itsTray->showMessage(QString::fromUtf8("Information"),
                             QString(itsPort->portName()) +
                             QString::fromUtf8(" port") +
                             QString::fromUtf8(" is opened!\n") +
                             QString::fromUtf8("Baud rate: ") +
                             QString(QString::number(itsPort->baudRate())) +
                             QString("\n") +
                             QString::fromUtf8("Data bits: ") +
                             QString(QString::number(itsPort->dataBits())) +
                             QString("\n") +
                             QString::fromUtf8("Parity: ") +
                             QString(QString::number(itsPort->parity())));
        bPortStart->setEnabled(false);
        bPortStop->setEnabled(true);
        lRx->setStyleSheet("background: none; font: bold; font-size: 10pt");
    }
    else
    {
        itsTray->showMessage(QString::fromUtf8("Error"),
                             QString::fromUtf8("Error opening port: ") +
                             QString(itsPort->portName()),
                             QSystemTrayIcon::Critical);
        lRx->setStyleSheet("background: red; font: bold; font-size: 10pt");
    }
}

void Dialog::closePort()
{
    itsPort->close();
    itsBlinkTimeNone->stop();
    itsBlinkTimeColor->stop();
    lRx->setStyleSheet("background: red; font: bold; font-size: 10pt");
    bPortStop->setEnabled(false);
    bPortStart->setEnabled(true);
    itsSensorProtocol->resetProtocol();
}

void Dialog::cbPortChanged()
{
    bPortStart->setEnabled(true);
    bPortStop->setEnabled(false);
}

void Dialog::received(bool isReceived)
{
    if(isReceived) {
        if(!itsBlinkTimeColor->isActive() && !itsBlinkTimeNone->isActive()) {
            itsBlinkTimeColor->start();
            lRx->setStyleSheet("background: green; font: bold; font-size: 10pt");
        }

        if(!itsTimeToDisplay->isActive()) {
            itsTimeToDisplay->start();
        }

        QList<QString> strKeysList = itsSensorProtocol->getReadedData().keys();
        for(int i = 0; i < itsSensorProtocol->getReadedData().size(); ++i) {
            itsTempSensorsList.append(itsSensorProtocol->getReadedData().value(strKeysList.at(i)));
        }
    }
}

void Dialog::setColorLCD(QLCDNumber *lcd, bool isHeat)
{
    QPalette palette;
    // get the palette
    palette = lcd->palette();
    if(isHeat) {
        // foreground color
        palette.setColor(palette.WindowText, QColor(100, 0, 0));
        // "light" border
        palette.setColor(palette.Light, QColor(100, 0, 0));
        // "dark" border
        palette.setColor(palette.Dark, QColor(100, 0, 0));
    } else {
        // foreground color
        palette.setColor(palette.WindowText, QColor(0, 0, 100));
        // "light" border
        palette.setColor(palette.Light, QColor(0, 0, 100));
        // "dark" border
        palette.setColor(palette.Dark, QColor(0, 0, 100));
    }
    // set the palette
    lcd->setPalette(palette);
}

QString &Dialog::addTrailingZeros(QString &str, int prec)
{
    if(str.isEmpty() || prec < 1) { // if prec == 0 then it's no sense
        return str;
    }

    int pointIndex = str.indexOf(".");
    if(pointIndex == -1) {
        str.append(".");
        pointIndex = str.size() - 1;
    }

    if(str.size() - 1 - pointIndex < prec) {
        int size = str.size();
        for(int i = 0; i < prec - (size - 1 - pointIndex); ++i) {
            str.append("0");
        }
    }

    return str;
}

void Dialog::colorIsRx()
{
    lRx->setStyleSheet("background: none; font: bold; font-size: 10pt");
    itsBlinkTimeColor->stop();
    itsBlinkTimeNone->start();
}

void Dialog::colorNoneRx()
{
    itsBlinkTimeNone->stop();
}

void Dialog::display()
{
    itsTimeToDisplay->stop();

    QList<QLCDNumber*> list;
    list << lcdSensor2Termo << lcdSensor1Termo << lcdCPUTermo;
    QString tempStr;

    for(int k = 0; k < list.size(); ++k) {
        tempStr = itsTempSensorsList.at(itsTempSensorsList.size() - 1 - k);

        if(list.at(k)->digitCount() < addTrailingZeros(tempStr, PRECISION).size())
        {
            list[k]->display("ERR"); // Overflow
        } else {
            list[k]->display(addTrailingZeros(tempStr, PRECISION));
        }

        setColorLCD(list[k], tempStr.toDouble() > 0.0);
#ifdef DEBUG
        qDebug() << "itsTempSensorsList.size() =" << itsTempSensorsList.size();
        qDebug() << "Temperature[" << k << "] =" << list.at(k)->value();
#endif
    }

    itsTempSensorsList.clear();
}
