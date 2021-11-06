#include "mainwindow.h"
#include "appconfig.h"
#include "applog.h"
#include <QApplication>

#include <unistd.h>

int main_recorder(int argc, char* argv[])
{
    QApplication a(argc, argv);
    //! файл "Simple Video HUB.conf" находится в папке "/home/pi/.config/NIKTES"
    appConfig::init("NIKTES", "Simple Video HUB");
    QString logFileName;
    int logLevel = 0;

#ifdef DESKTOP_DEBUG_BUILD
    QString raspFolder;
#ifdef Q_OS_LINUX
    raspFolder = "~/rasp_media/";
#endif
    raspFolder = "t:/rasp_media/";
#ifdef Q_OS_WIN

#endif

    logFileName = raspFolder + "log/vhub.log";
    appConfig::setValue("VLOG/MountPoint", raspFolder);
    appConfig::setValue("VLOG/Folder", "medialog");
    appConfig::setValue("COMMON/video_length", 2);
#else
    logFileName = appConfig::value("TLOG/FILE").toString();
    logLevel = appConfig::value("TLOG/LEVEL").toInt();
#endif

    appLog::init(logFileName, logLevel);
    MainWindow w;
#if defined (DESKTOP_DEBUG_BUILD)
    w.show();
#else
    w.showFullScreen();
#endif
    int res = a.exec();
    appLog::deinit();
    return res;
}


int main(int argc, char* argv[])
{

    return main_recorder(argc, argv);
}
