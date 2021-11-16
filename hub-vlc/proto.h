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
}PCK_Header_t;


typedef struct
{
    quint8 camId;
}PCK_CAM_t;


typedef struct
{
    quint8   camId;
    quint8   fanState;
    quint16  temper; //x1000 celsius degrees
}PCK_STATE_t;

#pragma pack()


enum PCK_Type
{
    PCT_SHUTDOWN = 0,
    PCT_CAM_SWITCH,
    PCT_STATE,
    PCT_UPDATE_EXECUTALE
};


QByteArray makePck(quint8 type, QByteArray data);


#endif // PROTO_H
