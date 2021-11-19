#ifndef RECVPARSER_HPP
#define RECVPARSER_HPP

#include <QObject>
#include <QIODevice>
#include <QMap>
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
    void shutdown();
protected:
    void  handleRecv(const QByteArray& rxData);
    const PCK_Header_t* hasPacket();
    const PCK_Header_t* checkCompletePacket();
    void removePacket();
private slots:
    void readyRead();
private:
    void onCamSwitch(const PCK_Header_t* hdr);
    void onShutdown(const PCK_Header_t* hdr);


    QIODevice* m_io = nullptr;
    QByteArray m_buffer;
    quint8 m_devId = 0;
    quint8 m_signature = CU_SIGNATURE_;
    using  packet_handler_f = std::function<void(const PCK_Header_t*)>;
    using  PacketHandlers = QMap<quint8, packet_handler_f>;
    PacketHandlers m_handlers;


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
