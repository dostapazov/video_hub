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

    logFileName = appConfig::get_log_name();
    logLevel = appConfig::get_log_level();

    appLog::init(logFileName, logLevel);
    MainWindow w;
    int res = a.exec();
    appLog::deinit();
    return res;
}

int main(int argc, char* argv[])
{

    return main_recorder(argc, argv);
}
