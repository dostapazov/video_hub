#include "mainwindow.h"
#include "appconfig.h"
#include "applog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //! файл "Simple Video HUB.conf" находится в папке "/home/pi/.config/NIKTES"
    appConfig::init("NIKTES", "Simple Video HUB");

#ifndef __linux__
//      appConfig::setValue("TLOG/FILE","d:/vhub.log");
//      appConfig::setValue("TLOG/LEVEL",0);
//      appConfig::setValue("VLOG/MountPoint","d:/rasp_media");
//      appConfig::setValue("VLOG/Folder","medialog");
    appConfig::setValue("COMMON/video_length",2);
#endif

    appLog::init(appConfig::value("TLOG/FILE").toString(),appConfig::value("TLOG/LEVEL").toUInt());

    MainWindow w;
#ifdef QT_DEBUG
    w.show();
#else
    w.showFullScreen();
#endif
    int res = a.exec();
    appLog::deinit();
    return res;
}
