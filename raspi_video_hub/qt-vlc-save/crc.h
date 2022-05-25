#ifndef CRC_H
#define CRC_H
#include <QtCore>

quint16 rtuCRC16(const char* data, quint16 len);
quint32 crc32(const QByteArray& data, int start = 0, int len = 0);

#endif // CRC_H
