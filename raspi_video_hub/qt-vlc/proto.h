#ifndef PROTO_H
#define PROTO_H
#include <QtCore>
#include "crc.h"

#define CU_SIGNATURE_  0x5a
#define RP_SIGNATURE_  0xa5

enum PCK_Type
{
    PCT_SHUTDOWN = 0,
    PCT_CAM_SWITCH,
    PCT_DATETIME,
    PCT_STATE,
    PCT_UPDATE_EXECUTALE
};


#pragma pack(push,1)

typedef struct
{
    quint8  signture;
    quint8  devId;
    quint8  pckType;
    quint8 size;
} PCK_Header_t;


typedef struct
{
    quint8 camId;
} PCK_CAM_t;


typedef struct
{
    quint8   camId;
    quint8   fanState;
    quint16  temper; //x1000 celsius degrees
} PCK_STATE_t;

typedef struct
{
    quint8     sec;
    quint8     min;
    quint8     hour;
    quint8     day;
    quint8     mounth;
    quint8     year;
} PCK_DateTime_t;

#pragma pack(pop)


constexpr int EMPTY_PACKET_SIZE = (sizeof(PCK_Header_t) + sizeof(quint32));
inline int packetSize(const PCK_Header_t* hdr) {return  EMPTY_PACKET_SIZE + hdr->size;}
QByteArray makePck(quint8 type, quint8 devId, QByteArray data);
bool checkCRC(const QByteArray& packet);


#endif // PROTO_H
