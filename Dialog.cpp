#include "Dialog.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPixmap>
#include <QGridLayout>
#include <QPainter>
#include <QImage>
#include <QMessageBox>
#include <QApplication>
#include <QFile>
#include <QDesktopWidget>
#include <QSystemTrayIcon>
#include <QShortcut>
#include <QSerialPortInfo>

#define LOWFREQ 4400
#define HIGHFREQ 5000
#define CHANELS 600
#define BYTESLENTH 8

Dialog::Dialog(QWidget *parent) :
        QDialog(parent),
        lPort(new QLabel(QString::fromUtf8("Port"), this)),
        cbPort(new QComboBox(this)),
        bPortOpen(new QPushButton(QString::fromUtf8("Open"), this)),
        lLogoPix(new QLabel(this)),
        lCPUTermoPix(new QLabel(this)),
        lSensor1TermoPix(new QLabel(this)),
        lSensor2TermoPix(new QLabel(this)),
        lCPUTermo(new QLabel("-50.12", this)),
        lSensor1Termo(new QLabel("-50.12", this)),
        lSensor2Termo(new QLabel("-50.12", this)),
        gbCPU(new QGroupBox(QString::fromUtf8("CPU"), this)),
        gbSensor1(new QGroupBox(QString::fromUtf8("Sensor 1"), this)),
        gbSensor2(new QGroupBox(QString::fromUtf8("Sensor 2"), this)),
        itsPort(new QSerialPort(this)),
        itsOnePacket(new OnePacket(itsPort, 5, 8, 1, 250, 300, 0, this)),
        itsTray (new QSystemTrayIcon(QPixmap(":/TermoViewIcon.png"), this))
{
    setLayout(new QVBoxLayout(this));

    // помещаю логотип фирмы
    QPixmap pixmapLogo(":/elisat.png");
    lLogoPix->setPixmap(pixmapLogo);
    lLogoPix->setMaximumSize(60, 16);
    lLogoPix->setMinimumSize(60, 16);
    lLogoPix->setScaledContents(true);

    QPixmap pixmapTermo(":/Termo.png");
    lCPUTermoPix->setPixmap(pixmapTermo);
    lCPUTermoPix->setMaximumSize(50, 50);
    lCPUTermoPix->setMinimumSize(50, 50);
    lCPUTermoPix->setScaledContents(true);

    lSensor1TermoPix->setPixmap(pixmapTermo);
    lSensor1TermoPix->setMaximumSize(50, 50);
    lSensor1TermoPix->setMinimumSize(50, 50);
    lSensor1TermoPix->setScaledContents(true);

    lSensor2TermoPix->setPixmap(pixmapTermo);
    lSensor2TermoPix->setMaximumSize(50, 50);
    lSensor2TermoPix->setMinimumSize(50, 50);
    lSensor2TermoPix->setScaledContents(true);

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(lPort, 0, 0);
    grid->addWidget(cbPort, 0, 1);
    grid->addWidget(bPortOpen, 0, 2);
    grid->addWidget(lLogoPix, 0, 3);
    grid->setSpacing(5);

    QVBoxLayout *verCPU = new QVBoxLayout;
    verCPU->addWidget(lCPUTermoPix, 0, Qt::AlignCenter);
    verCPU->addWidget(lCPUTermo, 0, Qt::AlignCenter);
    verCPU->setSpacing(5);

    QVBoxLayout *verSensor1 = new QVBoxLayout;
    verSensor1->addWidget(lSensor1TermoPix, 0, Qt::AlignCenter);
    verSensor1->addWidget(lSensor1Termo, 0, Qt::AlignCenter);
    verSensor1->setSpacing(5);

    QVBoxLayout *verSensor2 = new QVBoxLayout;
    verSensor2->addWidget(lSensor2TermoPix, 0, Qt::AlignCenter);
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

    itsTray->setVisible(true);

    connect(bPortOpen, SIGNAL(clicked()), this, SLOT(openPort()));
    connect(cbPort, SIGNAL(currentIndexChanged(int)), this, SLOT(cbPortChanged()));
    connect(itsOnePacket, SIGNAL(DataIsReaded(bool)), this, SLOT(answer()));

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
    itsPort->setBaudRate(QSerialPort::Baud38400);
    itsPort->setDataBits(QSerialPort::Data8);
    itsPort->setParity(QSerialPort::NoParity);
    itsPort->setFlowControl(QSerialPort::NoFlowControl);
    itsPort->open(QSerialPort::ReadOnly);

    if(itsPort->isOpen())
    {
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

void Dialog::answer()
{

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
