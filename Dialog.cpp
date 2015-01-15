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

#define ACCURACY 0.05

#define FORMAT 'f'
#define PRECISION 2

Dialog::Dialog(QWidget *parent) :
        QDialog(parent),
        lPort(new QLabel(QString::fromUtf8("Port"), this)),
        cbPort(new QComboBox(this)),
        lBaud(new QLabel(QString::fromUtf8("Baud"), this)),
        cbBaud(new QComboBox(this)),
        bPortOpen(new QPushButton(QString::fromUtf8("Open"), this)),
        lRx(new QLabel("   Rx   ", this)),
        lcdCPUTermo(new QLCDNumber(this)),
        lcdSensor1Termo(new QLCDNumber(this)),
        lcdSensor2Termo(new QLCDNumber(this)),
        gbCPU(new QGroupBox(QString::fromUtf8("CPU"), this)),
        gbSensor1(new QGroupBox(QString::fromUtf8("Sensor 1"), this)),
        gbSensor2(new QGroupBox(QString::fromUtf8("Sensor 2"), this)),
        itsPort(new QSerialPort(this)),
        itsOnePacket(new OnePacket(itsPort, STARTBYTE, STOPBYTE, BYTESLENTH, this)),
        itsPrevCPUTemp(0.0),
        itsPrevSensor1Temp(0.0),
        itsPrevSensor2Temp(0.0),
        itsTray (new QSystemTrayIcon(QPixmap(":/TermoViewIcon.png"), this)),
        itsBlinkTime(new QTimer(this))
{
    setLayout(new QVBoxLayout(this));

    lRx->setStyleSheet("background: none; font: bold; font-size: 10pt");
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
    grid->addWidget(bPortOpen, 2, 1);
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

    // делаю так, чтобы форма появлялась в центре экрана
//    this->move(qApp->desktop()->availableGeometry(this).center()-this->rect().center());

    // чтобы вызывался деструктор явно!!!
//    if (!testAttribute(Qt::WA_DeleteOnClose))
//        setAttribute(Qt::WA_DeleteOnClose, false);
    // false, чтобы не выдавало ошибку munmap_chunk(): invalid pointer

    QStringList portsNames;

    foreach(QSerialPortInfo portsAvailable, QSerialPortInfo::availablePorts())
    {
        portsNames << portsAvailable.portName();
    }

    cbPort->addItems(portsNames);
    cbPort->setEditable(false);

    QStringList portsBauds;
    portsBauds << "38400" << "57600" << "115200";
    cbBaud->addItems(portsBauds);
    cbPort->setEditable(false);


    itsTray->setVisible(true);
    itsBlinkTime->setInterval(100);

    QList<QLCDNumber*> list;
    list << lcdCPUTermo << lcdSensor1Termo << lcdSensor2Termo;
    foreach(QLCDNumber *lcd, list) {
        lcd->setMinimumSize(80, 40);
        lcd->setDigitCount(6);
        lcd->setSegmentStyle(QLCDNumber::Flat);
        lcd->setFrameStyle(QFrame::NoFrame);
    }

    connect(bPortOpen, SIGNAL(clicked()), this, SLOT(openPort()));
    connect(cbPort, SIGNAL(currentIndexChanged(int)), this, SLOT(cbPortChanged()));
    connect(cbBaud, SIGNAL(currentIndexChanged(int)), this, SLOT(cbPortChanged()));
    connect(itsOnePacket, SIGNAL(ReadedData(QByteArray)), this, SLOT(received(QByteArray)));
    connect(itsBlinkTime, SIGNAL(timeout()), this, SLOT(blinkRx()));

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
            itsPort->setBaudRate(QSerialPort::Baud38400);
            break;
        case 1:
            itsPort->setBaudRate(QSerialPort::Baud57600);
            break;
        case 2:
            itsPort->setBaudRate(QSerialPort::Baud115200);
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
        bPortOpen->setEnabled(false);
    }
    else
    {
        itsTray->showMessage(QString::fromUtf8("Error"),
                             QString::fromUtf8("Error opening port: ") +
                             QString(itsPort->portName()),
                             QSystemTrayIcon::Critical);
    }
}

void Dialog::cbPortChanged()
{
    bPortOpen->setEnabled(true);
}

void Dialog::received(QByteArray ba)
{
    lRx->setStyleSheet("background: red; font: bold; font-size: 10pt");
    itsBlinkTime->start();

    QList<QLCDNumber*> list;
    list << lcdCPUTermo << lcdSensor1Termo << lcdSensor2Termo;

    QString tempStr;

    for(int i = 1, k = 0, sensor = static_cast<int>(CPU); i < ba.size() - 1; i += 2, ++k, ++sensor) {
        if(sensor != static_cast<int>(CPU)) {
            tempStr = QString::number(tempCorr(tempSensors(wordToInt(ba.mid(i, 2))), static_cast<SENSORS>(sensor)), FORMAT, PRECISION);
        } else {
            tempStr = QString::number(tempCorr(tempCPU(wordToInt(ba.mid(i, 2))), CPU), FORMAT, PRECISION);
        }
        if(list.at(k)->digitCount() < addTrailingZeros(tempStr, PRECISION).size())
        {
            list[k]->display("ERR"); // Overflow
        } else {
            list[k]->display(addTrailingZeros(tempStr, PRECISION));
        }

        setColorLCD(list[k], tempStr.toDouble() > 0.0);
#ifdef DEBUG
        qDebug() << "Temperature[" << k << "] =" << list.at(k)->value();
#endif
    }
}

// преобразует word в byte
int Dialog::wordToInt(QByteArray ba)
{
    if(ba.size() != 2)
        return -1;

    int temp = ba[0];
    if(temp < 0)
    {
        temp += 0x100; // 256;
        temp *= 0x100;
    }
    else
        temp = ba[0]*0x100; // старший байт

    int i = ba[1];
    if(i < 0)
    {
        i += 0x100; // 256;
        temp += i;
    }
    else
        temp += ba[1]; // младший байт

    return temp;
}

// преобразование byte в word
QByteArray Dialog::toWord(int nInt)
{
    QByteArray ba;
    ba.resize(2);   //можно не писать!
    ba[0]=nInt%256; // младший байт
    ba[1]=nInt/256; // старший байт
    swapBytes(ba);  // если необходимо! Или можно поменять местами
    // индексы в двух верхних строчках

    return ba;
}

// меняет байты местами
void Dialog::swapBytes(QByteArray &ba)
{
    QByteArray temp;
    temp.resize(1); // можно не писать!
    temp[0]=ba[0];
    ba[0]=ba[1];
    ba[1]=temp[0];
}

// Преобразует hex вида 000000 в 00 00 00
QString Dialog::toHumanHex(QByteArray ba)
{
    QString str(' ');
    QByteArray baHex;
    baHex = ba.toHex().toUpper();
    for(int i = 0; i < baHex.size(); ++i)
    {
        if(i % 2)
            str = str + QString(baHex.at(i)) + ' ';
        else
            str = str + QString(baHex.at(i));
    }
    str.remove(0, 1);
    return str;
}

// Преобразует милисекунды в секунды
QString Dialog::mSecToSec(int time)
{
    int timeSec = time / 1000;
    int timeMSec = time % 1000;
    return (QString::number(timeSec) +
            QString::fromUtf8(".") +
            QString::number(timeMSec));
}

// определяет температуру
float Dialog::tempSensors(int temp)
{
    if(temp & NEGATIVE) {
        return -static_cast<float>(qAbs(temp - OFFSET))/SLOPE;
    } else {
        return static_cast<float>(temp)/SLOPE;
    }
}

// определяет температуру кристалла
float Dialog::tempCPU(int temp)
{
    return (static_cast<float>(temp*CPU_FACTOR - CPU_OFFSET))/CPU_SLOPE;
}

float Dialog::tempCorr(float temp, SENSORS sensor)
{
    float prevValue = 0.0;

    switch (sensor) {
    case CPU:
        prevValue = itsPrevCPUTemp;
#ifdef DEBUG
        qDebug() << "In CPU";
#endif
        break;
    case SENSOR1:
        prevValue = itsPrevSensor1Temp;
#ifdef DEBUG
        qDebug() << "In SENSOR1";
#endif
        break;
    case SENSOR2:
        prevValue = itsPrevSensor2Temp;
#ifdef DEBUG
        qDebug() << "In SENSOR2";
#endif
        break;
    default:
        prevValue = itsPrevCPUTemp;
        break;
    }

    if(prevValue) {
        prevValue = prevValue*(1 - ACCURACY) + temp*ACCURACY;
    } else {
        prevValue = temp;
    }

    switch (sensor) {
    case CPU:
        itsPrevCPUTemp = prevValue;
#ifdef DEBUG
        qDebug() << "Out CPU";
#endif
        break;
    case SENSOR1:
        itsPrevSensor1Temp = prevValue;
#ifdef DEBUG
        qDebug() << "Out SENSOR1";
#endif
        break;
    case SENSOR2:
        itsPrevSensor2Temp = prevValue;
#ifdef DEBUG
        qDebug() << "Out SENSOR2";
#endif
        break;
    default:
        itsPrevCPUTemp = prevValue;
        break;
    }

    return prevValue;
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
        for(int i = 0; i < prec - (str.size() - 1 - pointIndex); ++i) {
            str.append("0");
        }
    }

    return str;
}

void Dialog::blinkRx()
{
    lRx->setStyleSheet("background: none; font: bold; font-size: 10pt");
}
