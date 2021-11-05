#ifndef CRC32_H
#define CRC32_H

#include <stdint.h>
#include <qbytearray.h>

/*static*/ void crc32_init(void);
uint32_t crc32_byte(uint32_t init_crc, const uint8_t *buf, int len);
quint32 crc32(const QByteArray & data, quint64 start, quint64 len);

#endif // CRC32_H
