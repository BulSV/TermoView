#ifndef IPROTOCOL_H
#define IPROTOCOL_H

#include <QObject>
#include <QMultiMap>
#include <QString>
#include <QByteArray>

class IProtocol : public QObject
{
    Q_OBJECT
public:
    virtual ~IProtocol() {}
    virtual void setDataToWrite(const QMultiMap<QString, QByteArray> &data) = 0;
    virtual QMultiMap<QString, QByteArray> getReadedData() const = 0;
};

#endif // IPROTOCOL_H
