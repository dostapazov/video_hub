#ifndef CAM_LOGGER_FFMPEG_H
#define CAM_LOGGER_FFMPEG_H


#include <QtCore/QObject>
#include <qthread.h>


class cam_logger_ffmpeg : public QThread
{
    Q_OBJECT
    explicit cam_logger_ffmpeg(QObject *parent = nullptr);
             cam_logger_ffmpeg(const cam_logger_ffmpeg & other):QThread(other.parent()){};
    cam_logger_ffmpeg & operator = (const cam_logger_ffmpeg & other);

public:
    explicit cam_logger_ffmpeg(const int _id, const QString _name ,const QString _mrl, QObject * parent = Q_NULLPTR );

    int      get_id    () const   {return   m_id;}
    const QString  get_name  () const   {return m_name;}
    const QString  get_mrl   () const   {return m_mrl;}
    void     set_mrl   (const QString & _mrl);
    bool     start_streaming (const QString _root_folder, int time_length);
    void     stop_streaming ();


signals:

public slots:
private :
         int      get_time_interval(const QDateTime &dtm);
         QString  get_file_name    (const QDateTime &dtm);
    virtual  void run() override final;
             int  execute_ffmpeg(int time_interval, const QString & file_name);
             int  wait_ffmeg_finished();

    int               m_id = -1;
    QString           m_name;
    QString           m_mrl;
    QString           m_storage_root;
    int               m_time_lenght;

};





#endif // CAM_LOGGER_FFMPEG_H
