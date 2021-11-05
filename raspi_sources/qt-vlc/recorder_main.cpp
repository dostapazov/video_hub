#include "mainwindow.h"
#include "appconfig.h"
#include "applog.h"
#include <QApplication>

#include <unistd.h>

int main_recorder(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //! файл "Simple Video HUB.conf" находится в папке "/home/pi/.config/NIKTES"
    appConfig::init("NIKTES", "Simple Video HUB");

#ifdef _WINDOWS_
//      appConfig::setValue("TLOG/FILE","d:/vhub.log");
//      appConfig::setValue("TLOG/LEVEL",0);
//      appConfig::setValue("VLOG/MountPoint","d:/rasp_media");
//      appConfig::setValue("VLOG/Folder","medialog");
    appConfig::setValue("COMMON/video_length",2);
#endif

    appLog::init(appConfig::value("TLOG/FILE").toString(),appConfig::value("TLOG/LEVEL").toInt());

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


int main(int argc, char *argv[])
{

    return main_recorder(argc,argv);

//  int ret_code;
//  do{
//     pid_t pid;
//     int rv;
//     pid =  argc < 2 ? fork() : 0;
//     if(pid == -1) return -1;
//     if(pid)
//     {
//       wait(&rv);
//       ret_code = WEXITSTATUS(rv);
//       qDebug()<<"Exit code " << ret_code;
//       if(ret_code == UPDATE_EXIT_CODE)
//       {
//           if(do_rename_recorder(argv[0]))
//           {
//            system("sudo reboot");
//            break;
//           }
//       }

//     }
//     else
//     return main_recorder(argc,argv);
//    }while(ret_code);
//  return ret_code;
}
