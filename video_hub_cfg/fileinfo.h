#ifndef FILEINFO_H
#define FILEINFO_H
#include <QtCore>

typedef union
{
    struct
    {
        quint16 build;
        quint16 release;
        quint16 minor;
        quint16 major;
    }byteData;
    quint64 longVer;
}verInfo;
//
//
//
QVariantMap stringFileInfoVar(QString file, QString block, QStringList vals);
verInfo fileVersion(QString file);
QString verIntToStr(quint64 verInt);


#endif // FILEINFO_H
