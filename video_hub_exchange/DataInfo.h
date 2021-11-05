#ifndef DATAINFO_H
#define DATAINFO_H
#include <qglobal.h>
#include <qbytearray.h>

#pragma pack(1)

struct DataInfo
{
    // структура заголовка сообщения
    uchar startByte = 0xA5; // стартовый байт
    uint16_t sessionNumber = 0x1122;    // номер сессии
    uint16_t packageNumber = 0; // номер пакета
    uint16_t versionNumber = 1; // номер версии протокола
    uchar packageType = 2;  // пакет данных
    uint16_t dataSize = 6;  // размер поля данных

    // структура данных сообщения
    uint16_t error = 0; // ошибки
    uchar channel1 = 0; // канал 1
    uchar channel2 = 0; // канал 2
    uchar channel3 = 0; // канал 3
    uchar channel4 = 0; // канал 4

    QByteArray toByteArray()
    {
        return QByteArray(reinterpret_cast<const char*>(this), sizeof(*this));
    }

    bool fromByteArray(QByteArray data)
    {
        if (data.size() < static_cast<int>(sizeof(*this)))
            return false;
        memcpy(this, data.constData(), sizeof(*this));
        return true;
    }
};

struct TimeInfo
{
    // структура заголовка сообщения
    uchar startByte = 0xA5; // стартовый байт
    uint16_t sessionNumber = 0x1122;    // номер сессии
    uint16_t packageNumber = 0; // номер пакета
    uint16_t versionNumber = 1; // номер версии протокола
    uchar packageType = 3;  // пакет синхронизации времени
    uint16_t dataSize = 8;  // размер поля данных

    // пакет синхронизации времени
    uint16_t msec = 0;
    uchar sec = 0;
    uchar min = 0;
    uchar hour = 0;
    uchar day = 0;
    uchar month = 0;
    uchar year = 0;

    QByteArray toByteArray()
    {
        return QByteArray(reinterpret_cast<const char*>(this), sizeof(*this));
    }

    bool fromByteArray(QByteArray data)
    {
        if (data.size() < static_cast<int>(sizeof(*this)))
            return false;
        memcpy(this, data.constData(), sizeof(*this));
        return true;
    }
};

#pragma pack()

#endif // DATAINFO_H
