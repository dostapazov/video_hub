#include "proto.h"
#include "appconfig.h"


QByteArray makePck(quint8 type, quint8 devId, QByteArray data, quint8 sig  )
{
    PCK_Header_t header;

    header.signture = sig;
    header.pckType  = type;
    header.devId    = devId;
    header.size     = data.count();

    quint32 crc = 0;

    QByteArray res;
    res.resize(sizeof(header) + header.size + sizeof(crc));

    memcpy(res.data(), &header, sizeof(header));
    memcpy(res.data() + sizeof(header), data.constData(), header.size);
    crc = crc32(res, 0, sizeof(header) + header.size);
    memcpy(res.data() + sizeof(header) + header.size, &crc, sizeof(crc));

    return res;
}

bool checkCRC(const QByteArray& packet)
{
    if (packet.size() < EMPTY_PACKET_SIZE)
        return false;

    const PCK_Header_t* hdr = reinterpret_cast<const PCK_Header_t*>(packet.data());
    if (packet.size() < packetSize(hdr))
        return false;

    const quint32* pcrc = reinterpret_cast<const quint32*>(packet.constData() + hdr->size + sizeof(*hdr));
    return crc32(packet, 0, sizeof(*hdr) + hdr->size) == pcrc[0];
}
