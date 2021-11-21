/*************************************************************
* The purpose of this stream is to walk around  the camera folders.
* and delete old video files to restore space for
* future records
* OStapenko D.V. 2018-10-31 NIKTES
***********************************************************/

#include "filedeleterthread.h"
#include <qstorageinfo.h>
#include <qdiriterator.h>
#include <qdebug.h>
#include "applog.h"


FileDeleterThread::FileDeleterThread(QObject * parent)
                  :QThread (parent)
{

}

void FileDeleterThread::run()
{
   int time_interval;
  #ifdef __linux__
   time_interval = 60*1000;
  #else
   time_interval = 1000;
  #endif
   if(m_stream_root.isEmpty()){ exit(-1); return;}

   int time_count     = 0;
   int sleep_interval = 200;
   while(!isInterruptionRequested())
   {
    if(time_count > time_interval)
    {
      if(need_free_space())
         do_free_space  ();
      time_count = 0;
    }
    msleep(sleep_interval);
    time_count +=sleep_interval;
   }
  exit(0);
}



bool  FileDeleterThread::need_free_space()
{
  bool ret (false);
  QStorageInfo storage_info(m_stream_root);
#ifndef __linux__
  appLog::write(5,QString("avail %1 free %2 total %3")
                    .arg(storage_info.bytesAvailable())
                    .arg(storage_info.bytesFree())
                    .arg(storage_info.bytesTotal())
                ) ;
#endif
    int   free_percent = (qint64(100)*storage_info.bytesAvailable())/storage_info.bytesTotal();

    ret = free_percent < m_keep_free_percent;
    return ret;
}


//Создать список папок м меньшей датой и размером максимального файла
void set_dir_sorting(QDir & dir)
{
    QDir::SortFlags sf;
    sf.setFlag(QDir::SortFlag::DirsFirst);
    sf.setFlag(QDir::SortFlag::Name);
    dir.setSorting(sf);

}

void  FileDeleterThread::do_scan_cam_folders (const QString & cam_path)
{
    QDir  cam_dir(cam_path);
    QDir::SortFlags sf;
    QFileInfoList entryes = cam_dir.entryInfoList(QDir::NoFilter,QDir::Name);
    QStringList filtr;
    filtr<<"*.mp4";

    foreach(QFileInfo finfo, entryes)
    {
       if(isInterruptionRequested()) return;
       if(finfo.isDir())
       {
         if(!finfo.baseName().trimmed().isEmpty())
         {

             QDir date_dir = cam_dir;
             date_dir.cd(finfo.baseName());
#ifndef __linux__
             QString str;
             str = " dir "+cam_dir.dirName() +" -->"+ date_dir.dirName();
             qDebug()<<str;
#endif
             QFileInfoList cam_files = date_dir.entryInfoList(filtr);
             if(cam_files.size())
               {
                 if(!cams.contains(cam_dir.dirName()))
                 {
                  folders.insert(date_dir.dirName());
                  cams.insert(cam_dir.dirName());
                 }
                 else return;
               }
               else
                date_dir.removeRecursively();
         }
       }
    }
}

int  FileDeleterThread::do_scan_cams ()
{
 // Заполняет список камер и список папок с видео файлами
   folders.clear();
   cams.clear();

   QDir dir(m_stream_root);
   set_dir_sorting(dir);
   QDirIterator dir_iter ( dir );

   while(dir_iter.hasNext() && !isInterruptionRequested())
   {
    QString path = dir_iter.next();
    if(path.endsWith('.')) continue;
     QFileInfo info(path);
     if(info.isDir())
       {
        do_scan_cam_folders(path);
       }
   }
   return this->folders.size();
}

void  FileDeleterThread::do_remove_files()
{
    QStringList lcams = cams.toList();
    lcams.sort();
    foreach(QString name,lcams)
    {
      QDir dir(this->m_stream_root);
      dir.cd(name);
      name = *folders.begin();
      if(dir.exists(name))
      {
       dir.cd(name);
       dir.removeRecursively();
#ifndef  __linux__
       qDebug()<<"Remove directory "<<dir.canonicalPath();
#endif
      }
    }

}

void  FileDeleterThread::do_free_space   ()
{
    while(!isInterruptionRequested() && need_free_space() && do_scan_cams())
    {
       do_remove_files();
    };
}

