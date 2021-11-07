/***********************************************
 * class to save video files
 * from media stream with defined time lenght
 * via libvlc (!working with VLC 3.0.4 !)
 * implementation
 *
 * OStapenko D. V. 2018-11-07 NIKTES
 *
 * *********************************************/

#include "cam_logger_vlc.h"
#include "qdatetime.h"
#include <qdir.h>
#include "applog.h"


#define VLC_VER_EVENT_WORK 4

cam_logger_vlc::cam_logger_vlc(const cam_params_t& aParams, QObject* parent )
    : QObject(parent)
{

    m_params = aParams;
    cuttimer.setSingleShot(true);
    connect(&cuttimer, &QTimer::timeout, this, &cam_logger_vlc::on_cuttimer_timeout, Qt::ConnectionType::QueuedConnection);
    QString     str  = vlc::vlc_instance::get_version();
    QStringList sl = str.split(".");
    is_event_method =  (sl.size() > 1 && sl.at(0).toInt() >= VLC_VER_EVENT_WORK) ? true : false;
    appLog::write(0, tr("player version %1  events ").arg(is_event_method ? "support" : "not support"));
}

cam_logger_vlc::~cam_logger_vlc()
{
    stopStreaming();
    QThread::msleep(200);
}

void        cam_logger_vlc::timerEvent       (QTimerEvent* event)
{
    if (event->timerId() == m_timer_id)
    {
        if (m_player)
        {
            libvlc_state_t ps =  m_player->get_state();
            if (ps != libvlc_state_t::libvlc_Playing )
            {
                if (++m_check_play_counter > 10)
                {
                    QString str = tr("%1 WRONG state %2").arg(get_name()).arg(ps);
                    appLog::write(0, str);
                    qDebug() << str;
                    m_player->stop();
                    m_file_timelen = 0;
                    if (!is_event_method)
                        on_cuttimer_timeout();
                    m_check_play_counter = 0;
                }
            }
            else
                m_check_play_counter = 0;
        }
    }
    else
        QObject::timerEvent(event);
}

int         cam_logger_vlc::get_time_interval(const QDateTime& dtm)
{
    //const int msec_in_hour = 3600*1000;
    const int msec_in_day  = 24 * 3600 * 1000;
    QTime tm1 = dtm.time();
    QTime tm2 = tm1;
    tm2 = tm2.addSecs(m_time_duration);

    if ( m_time_duration >= 3600 )
        tm2 = tm2.addSecs(-(tm2.minute() * 60));
    else
    {
        if (m_time_duration >= 120)
        {
            int mlen = m_time_duration / 60;
            div_t dt = div(tm2.minute(), mlen);
            tm2 = tm2.addSecs(-(dt.rem * 60));
        }
    }

    tm2 = tm2.addSecs(-tm2.second () );

    tm2 = tm2.addMSecs(-tm2.msec());
    int interval = tm2.msecsSinceStartOfDay() - tm1.msecsSinceStartOfDay();
    if (interval < 0)
        interval += msec_in_day;
    interval = qMin(abs(interval), this->m_time_duration * 1000);
    return interval;

}

QString     cam_logger_vlc::get_file_name(const QDateTime& dtm)
{
    QString spath    = tr("%1/%2/%3")
                       .arg(mStorageFolder)
                       .arg(get_name())
                       .arg(dtm.toString("yyyy-MM-dd"));
    QDir dir (spath);
    if (!dir.exists())
        dir.mkpath(spath);
    int counter = 0;

    auto get_file_name = [this, spath, dtm, counter]()
    {
        QString ret = QString("%1/%2_%3").arg(spath).arg(get_name()).arg(dtm.toString("yyyy-MM-dd_hh-mm-ss"));
        if (counter)
            ret += QString("_%").arg(counter);
        ret += ".mp4";
        return ret;
    };

    QString file_name;

    do
    {
        file_name = get_file_name();
    }
    while (QFile::exists(file_name));

    return file_name;
}



void     cam_logger_vlc::startStreaming(const QString folder, int timeDuration)
{
    mStorageFolder = folder;
    m_time_duration = timeDuration;

//    if ( m_params.disabled || m_player || get_mrl().isEmpty() )
//        return false;
//    m_player = new vlc::vlc_player();
//    if (m_player)
//    {
//        if (is_event_method)
//        {
//            connect(m_player, &vlc::vlc_player::player_event, this, &cam_logger_vlc::player_events, Qt::ConnectionType::QueuedConnection);
//            m_player->event_activate(libvlc_MediaPlayerPlaying, true);
//            m_player->event_activate(libvlc_MediaPlayerStopped, true);
//            m_player->event_activate(libvlc_MediaPlayerEncounteredError, true);
//            m_player->event_activate(libvlc_MediaPlayerEndReached, true);
//            m_player->stop();
//        }
//        else
//            on_cuttimer_timeout();
//        m_timer_id =  startTimer(1000);
//    }
//    return true;
}

void     cam_logger_vlc::stopStreaming       ()
{

    if (m_player)
    {
        m_player->disconnect();
        m_player->stop();
        if (m_player->has_media())
            delete m_player->set_media(Q_NULLPTR);
        delete m_player;
        m_player = Q_NULLPTR;
    }
    if (m_timer_id)
        killTimer(m_timer_id);
}

int     cam_logger_vlc::create_next_media()
{
    if (m_next_media != Q_NULLPTR)
    {
#ifdef QT_DEBUG
        Q_ASSERT_X(0, "create_next_media", "previos media instance not null");
#else
        appLog::write(2, tr(" %1 create_next_media previos media instance not null").arg(get_name()));
#endif
    }

    QDateTime dtm     = QDateTime::currentDateTime();
    m_file_timelen    = get_time_interval(dtm);
    QString file_name = get_file_name(dtm);
    QString str;

    m_next_media      = new vlc::vlc_media;
    if (m_next_media)
    {
        if (m_next_media->open_location(get_mrl().toLocal8Bit().constData()))
        {

            m_next_media->add_option(":no-audio");
            m_next_media->add_option(":no-overlay");
            m_next_media->add_option(":rtsp-timeout=5000");
            m_next_media->add_option(":sout-mp4-faststart");

            str = tr(":network-caching=%1").arg(m_network_caching);
            m_next_media->add_option(str.toLocal8Bit().constData());
            div_t t     = div(m_file_timelen, 1000);

            if (this->is_event_method)
            {
                str = tr(":stop-time=%1.%2").arg(t.quot).arg(t.rem, 3, 10, QLatin1Char('0'));
                m_next_media->add_option(str.toLocal8Bit().constData());
            }
//            str = tr(":sout=#standard{access=file, mux=ts,dst=%1}").arg(file_name);
//            m_next_media->add_option(str.toLocal8Bit().constData());
//            m_next_media->add_option(":demux=h264");
            str = tr("%1 create next media  interval %2.%3").arg(get_name()).arg(t.quot).arg(t.rem);
        }
        else
        {
            str = tr("%1 error open  ").arg(get_name()).arg(get_mrl());
        }
        appLog::write(0, str);
        qDebug() << str;
    }
    return m_file_timelen;
}

vlc::vlc_media*    cam_logger_vlc::get_next_media   ()
{
    if (!m_next_media)
        create_next_media();
    vlc::vlc_media*   ret = Q_NULLPTR;
    std::swap(ret, m_next_media);
    return ret;
}


void   cam_logger_vlc::player_events(const libvlc_event_t event)
{
    QString str;
    switch (event.type)
    {
        case libvlc_MediaPlayerPlaying      :
            str = tr("%1 player playing").arg(get_name());
            appLog::write(0, str);
            cuttimer.setInterval(m_file_timelen);
            cuttimer.start();
            break;

        case libvlc_MediaPlayerEndReached       :
            str = tr("%1 player end reached").arg(get_name());
            appLog::write(0, str);
            qDebug() << str;
            create_next_media();
            break;
        case libvlc_MediaPlayerStopped          :
            str = tr("%1 player stopped,replace media").arg(get_name());
            appLog::write(0, str);
            qDebug() << str;
            m_player->set_media(get_next_media())->deleteLater();
            m_player->play();
            break;
        case libvlc_MediaPlayerEncounteredError :
            str = tr("%1 player stopped, errors encountered %2").arg(get_name()).arg(m_player->get_last_errors().join(", "));
            appLog::write(0, str);
            qDebug() << str;
            break;
    }

    if (cuttimer.isActive())
        cuttimer.stop();
    if (m_file_timelen)
        cuttimer.start(m_file_timelen);

}

void cam_logger_vlc::on_cuttimer_timeout()
{
    if (cuttimer.isActive())
        cuttimer.stop ();

//  if(m_player->is_playing())
//      m_player->stop(100);
    vlc::vlc_media* old_media = m_player->set_media(get_next_media());

    if (m_file_timelen)
    {
        m_player->play();
        cuttimer.start(m_file_timelen + m_network_caching);
    }
    if (old_media)
        old_media->deleteLater();
}

