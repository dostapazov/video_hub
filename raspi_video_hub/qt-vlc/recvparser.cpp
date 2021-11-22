#include "recvparser.hpp"



RecvParser::RecvParser(QObject* parent)
    : QObject{parent}
{
    using pair = std::pair<quint8, packet_handler_f>;
    namespace p = std::placeholders;

    m_handlers = PacketHandlers(
    {
        pair(PCT_SHUTDOWN, std::bind(&RecvParser::onShutdown, this, p::_1)),
        pair(PCT_CAM_SWITCH, std::bind(&RecvParser::onCamSwitch, this, p::_1)),
        pair(PCT_DATETIME, std::bind(&RecvParser::onSetDateTime, this, p::_1)),
        pair(PCT_STATE, std::bind(&RecvParser::onAppState, this, p::_1))
    }
    );
}

void RecvParser::setIoDevice(QIODevice* io)
{
    if (io == m_io)
        return;

    if (m_io)
        m_io->disconnect(this);

    m_buffer.clear();
    m_io = io;

    if (m_io)
        connect(m_io, &QIODevice::readyRead, this, &RecvParser::readyRead);
}

void RecvParser::readyRead()
{
    while (m_io && m_io->bytesAvailable())
        handleRecv(m_io->readAll());
}

void  RecvParser::handleRecv(const QByteArray& rxData)
{
    m_buffer.append(rxData);
    const PCK_Header_t* hdr;
    while ((hdr = hasPacket()))
    {
        if (checkCRC(m_buffer))
        {
            if (hdr->devId == getDevId())
            {
                auto handler = m_handlers.find(hdr->pckType);
                if (handler != m_handlers.end())
                {
                    handler.value()(hdr);
                }
                else
                {
                    emit errorPacket(m_buffer.left(packetSize(hdr)));
                }

            }
            m_buffer.remove(0, packetSize(hdr));
        }
        else
        {
            emit errorPacket(m_buffer.left(packetSize(hdr)), true);
            m_buffer.remove(0, 1);
        }
    }
}


void RecvParser::SyncBuffer()
{
    int index = m_buffer.indexOf(m_signature);
    if (index)
    {
        if (index > 0)
            m_buffer.remove(0, index);
        else
            m_buffer.clear();
    }
}

const PCK_Header_t* RecvParser::hasPacket()
{
    SyncBuffer();
    if (m_buffer.size() <  EMPTY_PACKET_SIZE)
        return nullptr;

    const PCK_Header_t* hdr = reinterpret_cast<const PCK_Header_t*>(m_buffer.constData());

    if (m_buffer.size() < packetSize(hdr))
    {
        m_buffer.remove(0, 1);
        return hasPacket();
    }

    if (hdr->pckType >= PCK_Type::PCT_MAX_COMMAND || hdr->size > PAKET_MAX_DATA_SIZE)
    {
        m_buffer.remove(0, 1);
        return hasPacket();
    }

    return hdr;
}

void RecvParser::onCamSwitch(const PCK_Header_t* hdr)
{
    if (hdr->size == sizeof (quint8))
    {
        const quint8* p_cam_id = reinterpret_cast<const quint8*>(hdr) + sizeof(*hdr);
        quint8 cam_id = p_cam_id[0];
        if (cam_id)
            emit camSwitch(cam_id - 1);
        return;
    }
    emit errorPacket(m_buffer.left(packetSize(hdr)));
}

void RecvParser::onShutdown(const PCK_Header_t* hdr)
{
    if (!hdr->size)
        emit shutDown();
    else
        emit errorPacket(m_buffer.left(packetSize(hdr)));
}

void RecvParser::onAppState(const PCK_Header_t* hdr)
{
    if (!hdr->size)
        emit appState();
    else
        emit errorPacket(m_buffer.left(packetSize(hdr)));
}

QDateTime RecvParser::fromPacket(const PCK_DateTime_t* src)
{
    QDateTime dtm(
        QDate(src->year + 2000, src->mounth, src->day),
        QTime(src->hour, src->min, src->sec)
    );
    return dtm;
}

void RecvParser::onSetDateTime(const PCK_Header_t* hdr)
{
    const char* ptr = reinterpret_cast<const char*>(hdr);
    const PCK_DateTime_t* dtm = reinterpret_cast<const PCK_DateTime_t*>(ptr + sizeof (*hdr));
    if (hdr->size == sizeof (*dtm))
    {
        emit setDateTime(fromPacket(dtm));
    }
    else
        emit errorPacket(m_buffer.left(packetSize(hdr)));
}

