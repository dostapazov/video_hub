#include "recvparser.hpp"



RecvParser::RecvParser(QObject* parent)
    : QObject{parent}
{
    using pair = std::pair<quint8, packet_handler_f>;
    namespace p = std::placeholders;


    m_handlers = PacketHandlers(
    {
        pair(PCT_SHUTDOWN, std::bind(&RecvParser::onShutdown, this, p::_1)),
        pair(PCT_CAM_SWITCH, std::bind(&RecvParser::onCamSwitch, this, p::_1))
    }
    );
}

void RecvParser::setIoDevice(QIODevice* io)
{
    if (m_io)
        m_io->disconnect(this);
    m_io = io;

    m_buffer.clear();

    if (!m_io)
        return;

    connect(m_io, &QIODevice::readyRead, this, &RecvParser::readyRead);

}

void RecvParser::readyRead()
{
    if (!m_io)
        return;
    handleRecv(m_io->readAll());
}

void  RecvParser::handleRecv(const QByteArray& rxData)
{
    m_buffer.append(rxData);
    const PCK_Header_t* hdr;
    while ( (hdr = hasPacket()))
    {
        if (checkCRC(m_buffer) && hdr)
        {
            auto handler = m_handlers.find(hdr->pckType);
            if (handler != m_handlers.end())
                handler.value()(hdr);
        }
        m_buffer.remove(0, packetSize(hdr));

    }
}


const PCK_Header_t* RecvParser::hasPacket()
{
    int index = m_buffer.indexOf(m_signature);
    if (index)
    {
        if (index > 0)
            m_buffer.remove(0, index);
        else
            m_buffer.clear();
    }

    return  checkCompletePacket();
}

const PCK_Header_t* RecvParser::checkCompletePacket()
{
    if (m_buffer.size() <  EMPTY_PACKET_SIZE)
        return nullptr;
    const PCK_Header_t* hdr = reinterpret_cast<const PCK_Header_t*>(m_buffer.constData());

    if (m_buffer.size() < packetSize(hdr))
        return nullptr;
    return hdr;
}


void RecvParser::removePacket()
{
    const PCK_Header_t* packet = checkCompletePacket();
    if (packet)
        m_buffer.remove(0, packetSize(packet));
}

void RecvParser::onCamSwitch(const PCK_Header_t* hdr)
{
    if (hdr->size)
    {
        const quint8* cam_id = reinterpret_cast<const quint8*>(hdr) + sizeof(*hdr);
        emit camSwitch(cam_id[0]);
    }
}

void RecvParser::onShutdown(const PCK_Header_t* hdr)
{

}



