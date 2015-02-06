#ifndef IPROTOCOL_H
#define IPROTOCOL_H

#include <QObject>
#include <QMultiMap>
#include <QString>

class IProtocol : public QObject
{
    Q_OBJECT
public:
    IProtocol(QObject *parent = 0) : QObject(parent) {}
    virtual ~IProtocol() {}
    virtual void setDataToWrite(const QMultiMap<QString, QString> &data) = 0;
    virtual QMultiMap<QString, QString> getReadedData() const = 0;
public slots:
    virtual void writeData() = 0;
    virtual void resetProtocol() = 0;
signals:
    void DataIsReaded(bool);
    void DataIsWrited(bool);
};

#endif // IPROTOCOL_H
