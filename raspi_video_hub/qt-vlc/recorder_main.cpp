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

#ifdef Q_OS_WIN
	appConfig::setValue("TLOG/FILE", "t:/rasp_media/log/vhub.log");
	appConfig::setValue("TLOG/LEVEL", 0);
	appConfig::setValue("VLOG/MountPoint", "t:/rasp_media");
	appConfig::setValue("VLOG/Folder", "medialog");
	appConfig::setValue("COMMON/video_length", 2);
#endif

	appLog::init(appConfig::value("TLOG/FILE").toString(), appConfig::value("TLOG/LEVEL").toInt());

	MainWindow w;
#if defined (QT_DEBUG) || defined (Q_OS_WIN)
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
