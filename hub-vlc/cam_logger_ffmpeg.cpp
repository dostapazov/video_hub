/***********************************************
 * class to save video files
 * from media stream with defined time lenght
 * via FFMPEG
 * implementation
 *
 * OStapenko D. V. 2018-11-07 NIKTES
 *
 * *********************************************/
#include "cam_logger_ffmpeg.h"
#include <qdir.h>
#include <qdatetime.h>
#include <qprocess.h>
#include <QString>
#include <qdebug.h>


cam_logger_ffmpeg::cam_logger_ffmpeg(QObject *parent) : QThread(parent)
{
}

cam_logger_ffmpeg::cam_logger_ffmpeg(const int _id, const QString _name ,const QString _mrl,  QObject * parent )
           :QThread(parent)
           ,m_id(_id)
           ,m_name(_name)
           ,m_mrl (_mrl )
{
}


bool     cam_logger_ffmpeg::start_streaming (const QString _root_folder, int time_length)
{
    if(!isRunning())
    {
        m_storage_root = _root_folder;
        m_time_lenght  = time_length;
        start();
    }
   return isRunning();
}

void     cam_logger_ffmpeg::stop_streaming ()
{
  if(this->isRunning())
  {
    requestInterruption();
    wait(5000);
  }
}

void cam_logger_ffmpeg::run()
{

  //ffmpeg  -r 15 -rtsp_transport tcp -i rtsp://192.168.0.101:554/media/video1 -an -vcodec copy -y -t 00:00:10.10 d:\v1.mp4
  while(!isInterruptionRequested())
  {
      QDateTime     dtm  = QDateTime::currentDateTime();
      int time_interval  = get_time_interval(dtm);
      QString  file_name = get_file_name(dtm);
      execute_ffmpeg(time_interval,file_name);
  }
}


QString get_time_string(int input)
{
    int  hour , mins;
    div_t t;
    t =  div(input,3600000);
    hour = t.quot;
    t = div(t.rem,60000);
    mins = t.quot;
    t = div(t.rem,1000);
    return QString("%1:%2:%3.%4").arg(hour,2,10,QLatin1Char('0')).arg(mins,2,10,QLatin1Char('0')).arg(t.quot,2,10,QLatin1Char('0')).arg(t.rem,3,10,QLatin1Char('0'));

}

int         cam_logger_ffmpeg::execute_ffmpeg(int time_interval, const QString& file_name)
{

  QString program = "ffmpeg";
  #ifdef WIN32
    program = "c:/ffmpeg/bin/ffmpeg.exe";
  #endif

//  QString arguments = tr("-i %1 -y -t %2 -movflags +faststart %3")
//                      .arg(m_mrl)
//                      .arg(get_time_string(time_interval))
//                      .arg(file_name);
//  QStringList args = arguments.split(" ");
  QStringList args;
  args.append("-i");
  args.append(m_mrl);
  args.append("-movflags");
  args.append("+faststart");
  args.append("-y");

  args.append("-t");
  args.append(get_time_string(time_interval));
  args.append(file_name);


  QProcess ffmpeg;

  ffmpeg.start(program,args);
  if(!ffmpeg.waitForStarted(1000))
      qDebug()<<ffmpeg.errorString();
  int sleep_time = 500;
  do
  {
    time_interval-= sleep_time ;
    msleep(sleep_time );
  }while(time_interval> 0 && !isInterruptionRequested() && ffmpeg.state() == QProcess::Running  && !ffmpeg.waitForFinished(0));

  ffmpeg.waitForFinished(1000);
  msleep(100 );
  ffmpeg.close();
  return ffmpeg.exitCode();
}

int         cam_logger_ffmpeg::get_time_interval(const QDateTime &dtm)
{
      //const int msec_in_hour = 3600*1000;
      const int msec_in_day  = 24*3600*1000;
      QTime tm1 = dtm.time();
      QTime tm2 = tm1;
      tm2=tm2.addSecs(m_time_lenght);

      if( m_time_lenght >= 3600 )
          tm2=tm2.addSecs(-(tm2.minute()*60));

      tm2=tm2.addSecs(-tm2.second ()    );

      tm2=tm2.addMSecs(-tm2.msec());
      int interval = tm2.msecsSinceStartOfDay() - tm1.msecsSinceStartOfDay();
      if(interval<0)
          interval+= msec_in_day;
      interval = qMin(abs(interval),this->m_time_lenght*1000);
      return interval;

}

QString     cam_logger_ffmpeg::get_file_name(const QDateTime & dtm)
{
  QString spath    = tr("%1/%2/%3")
                    .arg(m_storage_root)
                    .arg(m_name)
                    .arg(dtm.toString("yyyy-MM-dd"));
      QDir dir (spath);
      if(!dir.exists())  dir.mkpath(spath);
      int counter = 0;

      auto get_file_name = [this,spath,dtm,counter]()
      {
        QString ret = QString("%1/%2_%3").arg(spath).arg(m_name).arg(dtm.toString("yyyy-MM-dd_hh-mm-ss"));
        if(counter) ret += QString("_%").arg(counter);
          ret += ".mp4";
          return ret;
      };

      QString file_name;

      do{
         file_name = get_file_name();
        }while(QFile::exists(file_name));

      return file_name;
}


