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

#include <QWidget>
#include <QMap>
#include <functional>
#include "vlcclasses.hpp"
#include <qtimer.h>



struct cam_params_t
{
    int id = 0;
    QString name;
    QString mrl;
    bool disabled = false;
};


class cam_logger_vlc : public QObject
{
    Q_OBJECT
    cam_logger_vlc(QObject* parent = Q_NULLPTR): QObject(parent) {}
    cam_logger_vlc(const cam_logger_vlc& other): QObject(other.parent()) {}
    cam_logger_vlc& operator = (const cam_logger_vlc&) {return *this;}

public:

    explicit       cam_logger_vlc(const cam_params_t& aParams, QObject* parent = nullptr);
    ~cam_logger_vlc();
    int      get_id    () const   {return   m_params.id;}
    const QString  get_name  () const   {return m_params.name;}
    const QString  get_mrl   () const   {return m_params.mrl;}
    void set_mrl(const QString& mrl);
    bool startMonitoring(QWidget* widget, const QString& mrl);
    bool startStreaming(const QString folder, int timeDuration);
    void stop();
    bool isStreaming() { return m_StreamingMode;}
    bool togglePlaying();


signals :
    void onStartMon();
    void onStopMon();
    void onError();
    void framesChanged(int frames);


private Q_SLOTS:

    void     player_events(const libvlc_event_t event);
    void     nextFile();
    void     playChecker();

private:

    using player_event_handler_t = std::function<void(vlc::vlc_player*)>;
    using PlayerEventHandlers = QMap<libvlc_event_e, player_event_handler_t>;
    PlayerEventHandlers playerHandlers;
    vlc::vlc_player* createPlayer();

    void initPlayerHandlers();
    void OnPlayerStopped(vlc::vlc_player* player);
    void OnPlayerPlaying(vlc::vlc_player* player);
    void startPlayWatchDog();

    int       get_time_interval(const QDateTime& dtm);
    QString   get_file_name    (const QDateTime& dtm);
    int       setupMediaForStreaming(vlc::vlc_media* media);
    void      removeEmptyPreviousFile();
    vlc::vlc_media*  create_media();


    void          releasePlayer();
    bool          isEventSupport();

    bool          m_StreamingMode = false;

    cam_params_t  m_params;

    QTimer        cutTimer;
    QTimer        playWatchdog;

    QString       m_StorageFolder;
    QString       m_CurrentFileName;
    int           m_file_timelen    = 0;
    int           m_network_caching = 300;
    int           m_time_duration = 0;

    int           m_displayedFrames  = 0;


    vlc::vlc_player*  m_player     = nullptr;
};

#endif // CAM_LOGGER_H
