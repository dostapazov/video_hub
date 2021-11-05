#ifndef CRC_H
#define CRC_H
#include <QtCore>

quint16 rtuCRC16(const char* data, quint16 len);
quint32 crc32(QByteArray data, quint64 start = 0, quint64 len = 0);

#endif // CRC_H
