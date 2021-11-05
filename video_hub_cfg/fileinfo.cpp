#include "fileinfo.h"
#include <QDebug>
#include <QDir>
#include <QByteArray>

#if defined(Q_OS_WIN)
#include "windows.h"
#include "winver.h"
#endif

#if defined(Q_OS_WIN)
QVariantMap strFileInfoW(QString file, QString block, QStringList vals)
{
    QVariantMap r;
    DWORD h;
    DWORD verSize=GetFileVersionInfoSize((const WCHAR*)QDir::toNativeSeparators(file).utf16(), &h);
    char *buf;
    buf=(char *)GlobalAlloc(GMEM_FIXED, verSize);
    if (GetFileVersionInfo((const WCHAR*)QDir::toNativeSeparators(file).utf16(),h,verSize,buf)!=0)
    {
        UINT len;
        ushort* verData = new ushort[verSize];
        for (int i = 0; i < vals.count(); i++)
        {
            QString subBlock = QString("\\StringFileInfo\\%1\\%2").arg(block).arg(vals[i]);
            if (VerQueryValue(buf,(const WCHAR*)subBlock.utf16(),(void**)&verData,&len))
            {
                QString strVal = QString::fromUtf16(verData,len-1);
                r.insert(vals[i],strVal);
            }
        }
        delete verData;
    }
    GlobalFree(buf);
    return r;
}
//
//
//
verInfo fileVersionW(QString file)
{
    verInfo res;
    res.byteData.major = res.byteData.minor = res.byteData.release = res.byteData.build = 0;
    DWORD h;
    DWORD verSize=GetFileVersionInfoSize((const WCHAR*)QDir::toNativeSeparators(file).utf16(), &h);
    char *buf;
    buf=(char *)GlobalAlloc(GMEM_FIXED, verSize);
    if (GetFileVersionInfo((const WCHAR*)QDir::toNativeSeparators(file).utf16(),h,verSize,buf)!=0)
    {
        UINT len;
        VS_FIXEDFILEINFO* lp;
        if (VerQueryValue(buf,(const WCHAR*)QString("\\").utf16(),(void**)&lp,&len))
        {
            res.byteData.major = lp->dwFileVersionMS >> 16;
            res.byteData.minor = lp->dwFileVersionMS & 0xffff;
            res.byteData.release = lp->dwFileVersionLS >> 16;
            res.byteData.build = lp->dwFileVersionLS & 0xffff;
        }
    }
    GlobalFree(buf);
    return res;
}

#endif


QVariantMap stringFileInfoVar(QString file, QString block, QStringList vals)
{
    QVariantMap res;
#if defined(Q_OS_WIN)
    res = strFileInfoW(file,block,vals);
#endif
    return res;
}

verInfo fileVersion(QString file)
{
    verInfo res;
#if defined(Q_OS_WIN)
    res = fileVersionW(file);
#endif
    return res;
}


QString verIntToStr(quint64 verInt)
{
    verInfo i;
    i.longVer = verInt;
    QString res = "";

    if (i.byteData.build ) res = QString(".%1").arg(i.byteData.build) + res;
    if (i.byteData.release || !res.isEmpty()) res = QString(".%1").arg(i.byteData.release) + res;
    res = QString(".%1").arg(i.byteData.minor) + res;
    res = QString("%1").arg(i.byteData.major) + res;

    return res;
}
