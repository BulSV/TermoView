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
        lFreq(new QLabel(QString::fromUtf8("Frequency"), this)),
        sbFreq(new QSpinBox(this)),
        bFreqSet(new QPushButton(QString::fromUtf8("Set"), this)),
        bSetInitialPosition(new QPushButton(QString::fromUtf8("Reset"), this)),
        gbInfo(new QGroupBox(QString::fromUtf8("Info"), this)),
        lSM1(new QLabel(this)),
        lSM2(new QLabel(this)),
        lSM3(new QLabel(this)),
        lSM4(new QLabel(this)),
        lSM5(new QLabel(this)),
        lCurrentFreq(new QLabel(QString::fromUtf8("Undefined"), this)),
        lLogo(new QLabel(this)),
        itsPort(new QSerialPort(this)),
        itsOnePacket(new OnePacket(itsPort, 5, 8, 1, 250, 300, 0, this)),        
        itsConfigFileName("TermoView.conf"),
        configSettings(itsConfigFileName, QSettings::IniFormat),
        itsErrorFileName("error.wav"),
        itsSoundError(itsErrorFileName),
        itsIsSetInit(false),
        itsTray (new QSystemTrayIcon(QPixmap(":/TermoViewIcon.png"), this))
{
    setLayout(new QVBoxLayout(this));

    // помещаю логотип фирмы
    QPixmap pixmap(":/elisat.png");
    lLogo->setPixmap(pixmap);    
    lLogo->setMaximumSize(200, 53);
    lLogo->setMinimumSize(200, 53);
    lLogo->setScaledContents(true);    

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(lPort, 0, 0);
    grid->addWidget(cbPort, 0, 1);
    grid->addWidget(bPortOpen, 0, 2);    
    grid->addWidget(lFreq, 1, 0);
    grid->addWidget(sbFreq, 1, 1);
    grid->addWidget(bFreqSet, 1, 2);
    grid->setSpacing(5);

    QHBoxLayout *horInfo1 = new QHBoxLayout;
    horInfo1->addWidget(new QLabel("SM1", this), 0, Qt::AlignCenter);
    horInfo1->addWidget(new QLabel("SM2", this), 0, Qt::AlignCenter);
    horInfo1->addWidget(new QLabel("SM3", this), 0, Qt::AlignCenter);
    horInfo1->addWidget(new QLabel("SM4", this), 0, Qt::AlignCenter);
    horInfo1->addWidget(new QLabel("SM5", this), 0, Qt::AlignCenter);
    horInfo1->setSpacing(5);

    QHBoxLayout *horInfo2 = new QHBoxLayout;
    horInfo2->addWidget(lSM1, 0, Qt::AlignCenter);
    horInfo2->addWidget(lSM2, 0, Qt::AlignCenter);
    horInfo2->addWidget(lSM3, 0, Qt::AlignCenter);
    horInfo2->addWidget(lSM4, 0, Qt::AlignCenter);
    horInfo2->addWidget(lSM5, 0, Qt::AlignCenter);
    horInfo2->setSpacing(5);

    QVBoxLayout *verInfo = new QVBoxLayout;
    verInfo->addItem(horInfo1);
    verInfo->addItem(horInfo2);
    verInfo->setSpacing(5);

    gbInfo->setLayout(verInfo);

    QGridLayout *gridDown = new QGridLayout;
    gridDown->addWidget(bSetInitialPosition, 0, 0, 0, 0, Qt::AlignLeft);
    gridDown->addWidget(new QLabel("Frequency of the filter", this), 0, 1);
    gridDown->addWidget(lCurrentFreq, 1, 1, Qt::AlignCenter);
    gridDown->setSpacing(5);

    layout()->addItem(grid);
    layout()->addWidget(lLogo);
    layout()->addWidget(gbInfo);
    layout()->addItem(gridDown);
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
    cbPort->setEditable(true);

    sbFreq->setRange(LOWFREQ, HIGHFREQ);    
    sbFreq->setSuffix(QString::fromUtf8(" MHz"));       

    QFile fileConfigSettings;
    if(!fileConfigSettings.exists(itsConfigFileName))
    {
        writeConfigSettings();
    }
    else
    {
        readSettings();
    }

    sbFreq->setEnabled(false);
    bFreqSet->setEnabled(false);
    bSetInitialPosition->setEnabled(false);

    lSM1->setStyleSheet("border-style: outset; border-width: 1px; border-color: black;");
    lSM2->setStyleSheet("border-style: outset; border-width: 1px; border-color: black;");
    lSM3->setStyleSheet("border-style: outset; border-width: 1px; border-color: black;");
    lSM4->setStyleSheet("border-style: outset; border-width: 1px; border-color: black;");
    lSM5->setStyleSheet("border-style: outset; border-width: 1px; border-color: black;");

    bSetInitialPosition->setStyleSheet("font: bold;");
    bFreqSet->setStyleSheet("font: bold;");
    lCurrentFreq->setStyleSheet("font: bold;");

    itsTray->setVisible(true);

    connect(bPortOpen, SIGNAL(clicked()), this, SLOT(openPort()));    
    connect(bFreqSet, SIGNAL(clicked()), this, SLOT(setFreqFVP()));
    connect(bSetInitialPosition, SIGNAL(clicked()), this, SLOT(InitialPosition()));    
    connect(cbPort, SIGNAL(currentIndexChanged(int)), this, SLOT(cbPortChanged()));   
    connect(itsOnePacket, SIGNAL(DataIsReaded(bool)), this, SLOT(answer()));
	
	QShortcut *aboutShortcut = new QShortcut(QKeySequence("F1"), this);
    connect(aboutShortcut, SIGNAL(activated()), qApp, SLOT(aboutQt()));
}

Dialog::~Dialog()
{
    writeConfigSettings();
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
        sbFreq->setEnabled(true);
        bFreqSet->setEnabled(true);
        bSetInitialPosition->setEnabled(true);
    }
    else
    {
        itsTray->showMessage(QString::fromUtf8("Error"),
                             QString::fromUtf8("Error opening port: ") +
                             QString(itsPort->portName()),
                             QSystemTrayIcon::Critical);        
    }
}

void Dialog::readSettings()
{
#if defined (Q_OS_UNIX)
    cbPort->setEditText(configSettings.value("Port", "/dev/pts/1").toString());    
#elif defined (Q_OS_WIN)
    cbPort->setEditText(configSettings.value("Port", "COM1").toString());    
#endif    
    sbFreq->setValue(QString(configSettings.value("Frequency", LOWFREQ).toString()).remove(4, 7).toInt());
}

void Dialog::writeConfigSettings()
{
    configSettings.clear();
    configSettings.setValue("Port", cbPort->currentText());    
    configSettings.setValue("Frequency", sbFreq->text());
}

void Dialog::setFreqFVP()
{
    itsIsSetInit = false;
    send(0x01, 0x00, sbFreq->value());
    lCurrentFreq->setStyleSheet("font: bold;");
    lCurrentFreq->setText(sbFreq->text());
}

void Dialog::InitialPosition()
{
    itsIsSetInit = true;
    send(0x00, 0x00, 0x00);
    lCurrentFreq->setStyleSheet("font: bold;");
    lCurrentFreq->setText("Initial position");
}

void Dialog::send(int mode, int device, int freq)
{
    if(freq)
    {
        freq -= LOWFREQ; // чтобы с нулевой частоты начинать, а не с 4400
    }
    clearInfo();
    sendData.clear();
    sendData.resize(BYTESLENTH);

    sendData[0] = 0x55; // STARTBYTE
    sendData[1] = mode; // Mode
    sendData[2] = device; // Device
    sendData[3] = toWord(freq)[0]; // Frequency High
    sendData[4] = toWord(freq)[1]; // Frequency Low
    sendData[5] = 0x00; // Steps High
    sendData[6] = 0x00; // Steps Low
    sendData[7] = 0xAA; // STOPBYTE

    QMultiMap<int, QByteArray> sendDataMap;
    sendDataMap.insert(0, sendData);    
}

void Dialog::cbPortChanged()
{        
    sbFreq->setEnabled(false);
    bFreqSet->setEnabled(false);
    bSetInitialPosition->setEnabled(false);
    bPortOpen->setEnabled(true);
}

void Dialog::answer()
{
//    QList<QByteArray> readList = itsManyPackets->getReadedRecalls().values(0);
//    int recalled = 0;
//    int flag = 0;
//    int coincidenceSM1 = 0b000;
//    int coincidenceSM2 = 0b000;
//    int coincidenceSM3 = 0b000;
//    int coincidenceSM4 = 0b000;
//    int coincidenceSM5 = 0b000;
//    int statistic = 0b000000000000000;

//#if defined (DEBUG)
//    qDebug() << "void Dialog::answer(): readList.size() =" << readList.size();
//#endif

//    for(int i = 0; i < readList.size(); ++i)
//    {
//        switch(readList.at(i).at(6))
//        {
//        case 1: if(!(flag & 1))
//            {
//                flag += 1;
//                recalled += 1;
//            }
//            else
//            {
//                coincidenceSM1 += 0b001;
//            }
//            break;
//        case 2: if(!(flag & 2))
//            {
//                flag += 2;
//                recalled += 2;
//            }
//            else
//            {
//                coincidenceSM2 += 0b001;
//            }
//            break;
//        case 3: if(!(flag & 4))
//            {
//                flag += 4;
//                recalled += 4;
//            }
//            else
//            {
//                coincidenceSM3 += 0b001;
//            }
//            break;
//        case 4: if(!(flag & 8))
//            {
//                flag += 8;
//                recalled += 8;
//            }
//            else
//            {
//                coincidenceSM4 += 0b001;
//            }
//            break;
//        case 5: if(!(flag & 16))
//            {
//                flag += 16;
//                recalled += 16;
//            }
//            else
//            {
//                coincidenceSM5 += 0b001;
//            }
//            break;
//        default: break;
//        }
//#if defined (DEBUG)
//        qDebug() << "void Dialog::answer(): readList.at(" << i << ").at(6)" << readList.at(i).toHex();
//        qDebug() << "void Dialog::answer(): flag =" << flag;
//        qDebug() << "void Dialog::answer(): recalled =" << recalled;
//#endif
//    }

//    statistic = (coincidenceSM5 << 12) | (coincidenceSM4 << 9) | (coincidenceSM3 << 6) | (coincidenceSM2 << 3) | coincidenceSM1;

//    recalledAnswer(recalled, statistic);

//#if defined (DEBUG)
//    qDebug() << "void Dialog::answer(bool isNoErrors): recalled =" << recalled;
//#endif
    QFile file;
    if(/*(recalled != 0b11111) && */file.exists(itsErrorFileName))
    {
        itsSoundError.play();
    }
}

void Dialog::recalledAnswer(int recalled, int statistic)
{
    if(0b00001 & recalled)
    {
        if(!itsIsSetInit)
            lSM1->setStyleSheet("background: green; border-style: outset; border-width: 1px; border-color: black;");
        else
            lSM1->setStyleSheet("background: yellow; border-style: outset; border-width: 1px; border-color: black;");
    }
    else
        lSM1->setStyleSheet("background: red; border-style: outset; border-width: 1px; border-color: black;");

    if(0b00010 & recalled)
    {
        if(!itsIsSetInit)
            lSM2->setStyleSheet("background: green; border-style: outset; border-width: 1px; border-color: black;");
        else
            lSM2->setStyleSheet("background: yellow; border-style: outset; border-width: 1px; border-color: black;");
    }
    else
        lSM2->setStyleSheet("background: red; border-style: outset; border-width: 1px; border-color: black;");

    if(0b00100 & recalled)
    {
        if(!itsIsSetInit)
            lSM3->setStyleSheet("background: green; border-style: outset; border-width: 1px; border-color: black;");
        else
            lSM3->setStyleSheet("background: yellow; border-style: outset; border-width: 1px; border-color: black;");
    }
    else
        lSM3->setStyleSheet("background: red; border-style: outset; border-width: 1px; border-color: black;");

    if(0b01000 & recalled)
    {
        if(!itsIsSetInit)
            lSM4->setStyleSheet("background: green; border-style: outset; border-width: 1px; border-color: black;");
        else
            lSM4->setStyleSheet("background: yellow; border-style: outset; border-width: 1px; border-color: black;");
    }
    else
        lSM4->setStyleSheet("background: red; border-style: outset; border-width: 1px; border-color: black;");

    if(0b10000 & recalled)
    {
        if(!itsIsSetInit)
            lSM5->setStyleSheet("background: green; border-style: outset; border-width: 1px; border-color: black;");
        else
            lSM5->setStyleSheet("background: yellow; border-style: outset; border-width: 1px; border-color: black;");
    }
    else
    {
        lSM5->setStyleSheet("background: red; border-style: outset; border-width: 1px; border-color: black;");
#if defined (DEBUG)
        qDebug() << "background: red";
#endif
    }
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

void Dialog::clearInfo()
{
    lSM1->setStyleSheet("border-style: outset; border-width: 1px; border-color: black;");
    lSM2->setStyleSheet("border-style: outset; border-width: 1px; border-color: black;");
    lSM3->setStyleSheet("border-style: outset; border-width: 1px; border-color: black;");
    lSM4->setStyleSheet("border-style: outset; border-width: 1px; border-color: black;;");
    lSM5->setStyleSheet("border-style: outset; border-width: 1px; border-color: black;");
}

