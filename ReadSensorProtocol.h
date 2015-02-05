#ifndef READSENSORPROTOCOL_H
#define READSENSORPROTOCOL_H

#include "IProtocol.h"
#include "ComPort.h"

class ReadSensorProtocol : public IProtocol
{
    Q_OBJECT
public:
    explicit ReadSensorProtocol(ComPort *comPort, QObject *parent = 0);
    virtual void setDataToWrite(const QMultiMap<QString, QString> &data);
    virtual QMultiMap<QString, QString> getReadedData() const;
signals:

public slots:
    virtual void readData(bool isReaded);
    virtual void writeData();
private slots:
    void resetReading(); // TODO link with Dialog?
private:
    ComPort *itsComPort;

    QMultiMap<QString, QString> itsReadData;

    float itsPrevCPUTemp;
    float itsPrevSensor1Temp;
    float itsPrevSensor2Temp;

    bool itsWasPrevCPUTemp;
    bool itsWasPrevSensor1Temp;
    bool itsWasPrevSensor2Temp;

    // датчики температуры
    enum SENSORS {
        CPU, SENSOR1, SENSOR2
    };

    // преобразует word в byte
    int wordToInt(QByteArray ba);
    // определяет температуру
    float tempSensors(int temp);
    // определяет температуру кристалла
    float tempCPU(int temp);
    // коррекция скачков температуры
    float tempCorr(float temp, SENSORS sensor);
    // преобразует enum SENSORS в строку
    QString sensorToString(SENSORS sensor);
};

#endif // READSENSORPROTOCOL_H
