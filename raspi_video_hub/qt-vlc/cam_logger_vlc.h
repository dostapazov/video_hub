/***********************************************
 * class to save video files
 * from media stream with defined time lenght
 * via libvlc (!working with VLC 3.0.4 !)
 * interface
 *
 * OStapenko D. V. 2018-11-07 NIKTES
 *
 * *********************************************/


#ifndef CAM_LOGGER_H
#define CAM_LOGGER_H

#include <QtCore/QObject>
#include "vlcclasses.hpp"
#include <qtimer.h>



class cam_logger_vlc : public QObject
{
   Q_OBJECT
    cam_logger_vlc(QObject * parent = Q_NULLPTR):QObject(parent){}
    cam_logger_vlc(const cam_logger_vlc & other):QObject(other.parent()){}
    cam_logger_vlc & operator = (const cam_logger_vlc &){return *this;}

  public:
    explicit       cam_logger_vlc(bool log_disable, const int m_id, const QString _name, const QString _mrl , const QString _opts = QString(), QObject * parent = nullptr);
                  ~cam_logger_vlc();
          int      get_id    () const   {return   m_id;}
    const QString  get_name  () const   {return m_name;}
    const QString  get_mrl   () const   {return m_mrl;}
          void     set_mrl   (const QString & _mrl);
          bool     start_streaming     (const QString _root_folder, int time_length);
          void     stop_streaming      ();

private Q_SLOTS:
          void     player_events(const libvlc_event_t event);
          void     on_cuttimer_timeout();
   private:
          void     timerEvent       (QTimerEvent * event);
          int      get_time_interval(const QDateTime &dtm);
       QString     get_file_name    (const QDateTime &dtm);
          int create_next_media();
vlc::vlc_media  *  get_next_media   ();
    QTimer            cuttimer;
    bool              is_event_method = false;
    int               m_id = -1;
    QString           m_name;
    QString           m_mrl;
    QString           m_options;
    QString           m_storage_root;
    int               m_file_timelen    = 0;
    int               m_network_caching = 300;
    int               m_time_lenght;
    int               m_timer_id  = 0;
    bool              m_log_diasble = false;
    int               m_check_play_counter = 0;


    vlc::vlc_player   * m_player     = nullptr;
    vlc::vlc_media    * m_next_media = nullptr;

};

struct cam_logger_less
{
    bool operator ()(const cam_logger_vlc & cm1, const cam_logger_vlc & cm2)
    {
      return cm1.get_id() < cm2.get_id();
    }

    bool operator ()(const cam_logger_vlc * const cm1, const cam_logger_vlc *const cm2)
    {
      if(cm1 && cm2 ) return (*this)(*cm1,*cm2);
      if(!cm1) return true;
      if(!cm2) return false;
    }
};


#endif // CAM_LOGGER_H
