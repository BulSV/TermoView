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

#define STARTBYTE 0x55
#define STOPBYTE 0xAA
#define BYTESLENTH 8

#define NEGATIVE 32768 // 2^15
#define OFFSET 45536
#define SLOPE 128
#define FORMAT 'f'
#define PRECISION 2

Dialog::Dialog(QWidget *parent) :
        QDialog(parent),
        lPort(new QLabel(QString::fromUtf8("Port"), this)),
        cbPort(new QComboBox(this)),
        lBaud(new QLabel(QString::fromUtf8("Baud"), this)),
        cbBaud(new QComboBox(this)),
        bPortOpen(new QPushButton(QString::fromUtf8("Open"), this)),
        lCPUTermo(new QLabel("NONE", this)),
        lSensor1Termo(new QLabel("NONE", this)),
        lSensor2Termo(new QLabel("NONE", this)),
        gbCPU(new QGroupBox(QString::fromUtf8("CPU"), this)),
        gbSensor1(new QGroupBox(QString::fromUtf8("Sensor 1"), this)),
        gbSensor2(new QGroupBox(QString::fromUtf8("Sensor 2"), this)),
        itsPort(new QSerialPort(this)),
        itsOnePacket(new OnePacket(itsPort, STARTBYTE, STOPBYTE, BYTESLENTH, this)),
        itsTray (new QSystemTrayIcon(QPixmap(":/TermoViewIcon.png"), this))
{
    setLayout(new QVBoxLayout(this));

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(lPort, 0, 0);
    grid->addWidget(cbPort, 0, 1);
    grid->addWidget(lBaud, 1, 0);
    grid->addWidget(cbBaud, 1, 1);
    // помещаю логотип фирмы
    grid->addWidget(new QLabel("<img src=':/elisat.png' height='16' width='60'/>", this), 0, 2);
    grid->addWidget(bPortOpen, 1, 2);
    grid->setSpacing(5);

    QVBoxLayout *verCPU = new QVBoxLayout;
    verCPU->addWidget(new QLabel("<img src=':/Termo.png' height='50' width='50'/>", this), 0, Qt::AlignCenter);
    verCPU->addWidget(lCPUTermo, 0, Qt::AlignCenter);
    verCPU->setSpacing(5);

    QVBoxLayout *verSensor1 = new QVBoxLayout;
    verSensor1->addWidget(new QLabel("<img src=':/Termo.png' height='50' width='50'/>", this), 0, Qt::AlignCenter);
    verSensor1->addWidget(lSensor1Termo, 0, Qt::AlignCenter);
    verSensor1->setSpacing(5);

    QVBoxLayout *verSensor2 = new QVBoxLayout;
    verSensor2->addWidget(new QLabel("<img src=':/Termo.png' height='50' width='50'/>", this), 0, Qt::AlignCenter);
    verSensor2->addWidget(lSensor2Termo, 0, Qt::AlignCenter);
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
    this->move(qApp->desktop()->availableGeometry(this).center()-this->rect().center());

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

    connect(bPortOpen, SIGNAL(clicked()), this, SLOT(openPort()));
    connect(cbPort, SIGNAL(currentIndexChanged(int)), this, SLOT(cbPortChanged()));
    connect(cbBaud, SIGNAL(currentIndexChanged(int)), this, SLOT(cbPortChanged()));
    connect(itsOnePacket, SIGNAL(ReadedData(QByteArray)), this, SLOT(answer(QByteArray)));

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

void Dialog::answer(QByteArray ba)
{
    QList<QLabel*> list;
    list << lCPUTermo << lSensor1Termo << lSensor2Termo;
    for(int i = 1, k = 0; i < ba.size() - 1; i += 2, ++k) {
        list[k]->setText(QString::number(temperature(wordToInt(ba.mid(i, 2))), FORMAT, PRECISION));
#ifdef DEBUG
        qDebug() << "Temperature[" << k << "] =" << list.at(k)->text();
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
float Dialog::temperature(int temp)
{
    if(temp & NEGATIVE) {
        return -static_cast<float>(qAbs(temp - OFFSET))/SLOPE;
    } else {
        return static_cast<float>(temp)/SLOPE;
    }
}
