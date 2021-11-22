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
    void setPacketSignature(quint8 sign);
    quint8 getPacketSignature();

    int  bufferSize() {return m_buffer.size();}


signals:
    void camSwitch(quint8);
    void shutDown();
    void appState();
    void setDateTime(QDateTime);
    void errorPacket(QByteArray, bool crcError = false);
protected:
    void  handleRecv(const QByteArray& rxData);
    void SyncBuffer();
    const PCK_Header_t* hasPacket();
    void removePacket();
private slots:
    void readyRead();
private:
    void onCamSwitch(const PCK_Header_t* hdr);
    void onShutdown(const PCK_Header_t* hdr);
    void onAppState(const PCK_Header_t* hdr);
    void onSetDateTime(const PCK_Header_t* hdr);
    static QDateTime fromPacket(const PCK_DateTime_t* src);


    QIODevice* m_io = nullptr;
    QByteArray m_buffer;
    quint8 m_devId = 0;
    quint8 m_signature = CU_SIGNATURE_;
    using  packet_handler_f = void (RecvParser::*)(const PCK_Header_t*);

    packet_handler_f m_handlers[PCT_MAX_COMMAND];


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

inline void RecvParser::setPacketSignature(quint8 sign)
{
    m_signature = sign;
}

inline quint8 RecvParser::getPacketSignature()
{
    return m_signature;
}



#endif // RECVPARSER_HPP
