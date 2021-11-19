#include "proto.h"
#include "appconfig.h"


QByteArray makePck(quint8 type, quint8 devId, QByteArray data)
{
    PCK_Header_t header;

    header.signture = RP_SIGNATURE_;
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
