#ifndef FILEDELETERTHREAD_H
#define FILEDELETERTHREAD_H

#include <QtCore/QObject>
#include <qthread.h>
#include "qmap.h"
#include "qset.h"

class FileDeleterThread : public QThread
{
    Q_OBJECT
public:
    FileDeleterThread         (QObject *parent = Q_NULLPTR );
    void set_stream_root      (QString streamRoot){m_stream_root = streamRoot;}
    void set_keep_free_percent(int val){m_keep_free_percent = qMax(3,val);}

private:

       virtual void  run() override final;
               bool  need_free_space();
               void do_remove_files();
               void  do_free_space  ();
               int   do_scan_cams();
               void  do_scan_cam_folders(const QString & cam_path);
       QString m_stream_root;
       int     m_keep_free_percent = 10;

       QSet<QString> folders;
       QSet<QString> cams;


};

#endif // FILEDELETERTHREAD_H
