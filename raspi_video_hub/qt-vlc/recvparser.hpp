#ifndef RECVPARSER_HPP
#define RECVPARSER_HPP

#include <QObject>
#include <QIODevice>
#include "proto.h"


class RecvParser : public QObject
{
    Q_OBJECT
public:
    explicit RecvParser(QObject* parent = nullptr);
    void setIoDevice(QIODevice* io);
    void setDevId(quint8 devId);
    quint8 getDevId();
    int  bufferSize() {return m_buffer.size();}


signals:
    void camSwitch(quint8);
protected:
    void  handleRecv(const QByteArray& rxData);
    const PCK_Header_t* hasPacket();
    const PCK_Header_t* checkCompletePacket();
    void removePacket();
private slots:
    void readyRead();
private:
    QIODevice* m_io = nullptr;
    QByteArray m_buffer;
    quint8 m_devId = 0;

};

inline void RecvParser::setDevId(quint8 devId)
{
    m_devId = devId;
    m_buffer.clear();
}

inline quint8 RecvParser::getDevId()
{
    return m_devId;
}


#endif // RECVPARSER_HPP
