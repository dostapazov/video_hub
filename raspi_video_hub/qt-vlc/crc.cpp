#include "crc.h"


quint32 crc32(const QByteArray& data, int start, int len)
{
    int i, j;
    quint32 byte, crc, mask;

    i = 0;
    crc = 0xFFFFFFFF;
    const char* beg = data.begin() + start;
    const char* end = beg + len;
    while (beg < end)
    {
        byte = *beg++;            // Get next byte.
        crc = crc ^ byte;
        for (j = 7; j >= 0; j--)      // Do eight times.
        {
            mask = -(crc & 1);
            crc = (crc >> 1) ^ (0xEDB88320 & mask);
        }
        i = i + 1;
    }
    crc = ~crc;
    return crc;
}
