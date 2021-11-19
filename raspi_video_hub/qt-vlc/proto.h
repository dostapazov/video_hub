#ifndef PROTO_H
#define PROTO_H
#include <QtCore>
#include "crc.h"

#define CU_SIGNATURE_  0x5a
#define RP_SIGNATURE_  0xa5

#pragma pack(1)
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
    uint8_t     sec;
    uint8_t     min;
    uint8_t     hour;
    uint8_t     day;
    uint8_t     mounth;
    uint8_t     year;
} PCK_DateTime_t;

#pragma pack()


enum PCK_Type
{
    PCT_SHUTDOWN = 0,
    PCT_CAM_SWITCH,
    PCT_DATETIME,
    PCT_STATE,
    PCT_UPDATE_EXECUTALE
};

constexpr int EMPTY_PACKET_SIZE = (sizeof(PCK_Header_t) + sizeof(quint32));
inline int packetSize(const PCK_Header_t* hdr) {return  EMPTY_PACKET_SIZE + hdr->size;}
QByteArray makePck(quint8 type, quint8 devId, QByteArray data);
bool checkCRC(const QByteArray& packet);


#endif // PROTO_H
