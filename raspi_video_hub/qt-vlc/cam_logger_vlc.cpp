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


cam_logger_vlc::cam_logger_vlc(const cam_params_t& aParams, QObject* parent )
    : QObject(parent)
{
    m_params = aParams;
    cutTimer.setSingleShot(true);
    connect(&cutTimer, &QTimer::timeout, this, &cam_logger_vlc::nextFile, Qt::ConnectionType::QueuedConnection);
    connect(&playWatchdog, &QTimer::timeout, this, &cam_logger_vlc::playChecker, Qt::ConnectionType::QueuedConnection);

    initPlayerHandlers();
}

cam_logger_vlc::~cam_logger_vlc()
{
    stop();
    QThread::msleep(200);
}

bool cam_logger_vlc::isEventSupport()
{
    constexpr int VLC_VER_EVENT_WORK = 3;
    QString     str  = vlc::vlc_instance::get_version();
    QStringList sl = str.split(".");
    return  (sl.size() > 1 && sl.at(0).toInt() >= VLC_VER_EVENT_WORK) ? true : false;
}

int         cam_logger_vlc::get_time_interval(const QDateTime& dtm)
{
    if (!m_time_duration)
        return 0;
    int interval = 2 * 60 * 1000;// 2 минуты
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

    auto get_uniq_name = [this, spath, dtm, counter]()
    {
        QString ret = QString("%1/%2_%3").arg(spath).arg(get_name()).arg(dtm.toString("yyyy-MM-dd_hh-mm-ss"));
        if (counter)
            ret += QString("_%1").arg(counter);
        ret += ".mp4";
        return ret;
    };

    QString file_name;

    do
    {
        file_name = get_uniq_name();
        ++counter;
    }
    while (QFile::exists(file_name));

    return file_name;
}


bool cam_logger_vlc::startMonitoring(QWidget* widget, const QString& mrl)
{

    if (!widget || mrl.isEmpty())
        return false;

    if (!m_player)
    {
        createPlayer();
    }
#ifdef Q_OS_LINUX
    m_player->set_drawable(widget->winId());
#else
    m_player->set_drawable((void*)widget->winId());
#endif

    m_params.mrl = mrl;
    vlc::vlc_media* media = m_player->set_media(create_media());

    m_player->play();
    if (media)
        media->deleteLater();
    startPlayWatchDog();
    return true;
}

bool cam_logger_vlc::startStreaming(const QString folder, int timeDuration)
{
    if (folder.isEmpty() || !timeDuration)
        return false;

    mStorageFolder = folder;
    m_time_duration = timeDuration ;
    nextFile();
    return true;
}

void     cam_logger_vlc::stop  ()
{
    releasePlayer();
}



int cam_logger_vlc::setupMediaForStreaming(vlc::vlc_media* media)
{
    QString str;
    QDateTime dtm     = QDateTime::currentDateTime();
    int time_len    = get_time_interval(dtm);
    if (time_len)
    {
        QString file_name = get_file_name(dtm);

        media->add_option(":no-audio");
        media->add_option(":no-overlay");
        media->add_option(":sout-mp4-faststart");

        str = tr(":network-caching=%1").arg(m_network_caching);
        media->add_option(str.toLocal8Bit().constData());


        str = tr(":sout=#standard{access=file, mux=ts,dst=%1}").arg(file_name);
        media->add_option(str.toLocal8Bit().constData());
        media->add_option(":demux=h264");
    }
    return time_len;
}

void cam_logger_vlc::set_mrl(const QString& mrl)
{
    if (m_params.mrl != mrl)
    {
        m_params.mrl = mrl;
        if (m_player && m_player->hasMedia())
        {
            m_player->open_mrl(mrl);
        }

    }
}

vlc::vlc_media*  cam_logger_vlc::create_media()
{
    QString str;
    m_file_timelen = 0;
    vlc::vlc_media* media   = new vlc::vlc_media;
    if (media)
    {
        media->add_option(":rtsp-timeout=5000");
        if (media->open_location(get_mrl().toLocal8Bit().constData()))
        {
            m_file_timelen = setupMediaForStreaming(media);
            div_t t     = div(m_file_timelen, 1000);
            str = tr("%1 create next media  interval %2.%3").arg(get_name()).arg(t.quot).arg(t.rem);
        }
        else
        {
            str = tr("%1 error open  ").arg(get_name()).arg(get_mrl());

        }
        appLog::write(0, str);
        qDebug() << str;
    }
    return media;
}


void cam_logger_vlc::nextFile()
{
    qDebug() << this->get_name() << "  -- next file";
    if (cutTimer.isActive())
        cutTimer.stop ();

    createPlayer();

    vlc::vlc_media* media = create_media();
    media = m_player->set_media(media);
    m_player->play();
    if (media)
        media->deleteLater();

}


void cam_logger_vlc::initPlayerHandlers()
{
    namespace p = std::placeholders;
    playerHandlers[libvlc_MediaPlayerPlaying] = std::bind(&cam_logger_vlc::OnPlayerPlaying, this, p::_1);
    playerHandlers[libvlc_MediaPlayerStopped] = std::bind(&cam_logger_vlc::OnPlayerStopped, this, p::_1);
}

void cam_logger_vlc::OnPlayerStopped(vlc::vlc_player* player)
{
    Q_UNUSED(player)
    if (isStreaming())
    {
        QString str = tr("%1 player stopped").arg(get_name());
        appLog::write(0, str);
        qDebug() << str;

    }
    else
    {
        emit onStopMon();
    }
}

void cam_logger_vlc::OnPlayerPlaying(vlc::vlc_player* player)
{
    Q_UNUSED(player)
    if (isStreaming())
    {
        QString str = tr("%1 player playing").arg(get_name());
        appLog::write(0, str);
        cutTimer.setInterval(m_file_timelen);
        cutTimer.start();
    }
    else
    {
        emit onStartMon();
    }


}


void   cam_logger_vlc::player_events(const libvlc_event_t event)
{
    player_event_handler_t handler = playerHandlers.value(static_cast<libvlc_event_e>(event.type));
    if (handler)
        handler(m_player);
}


vlc::vlc_player*   cam_logger_vlc::createPlayer()
{
    if (!m_player)
    {
        m_player = new vlc::vlc_player;
        connect(m_player, &vlc::vlc_player::player_event, this, &cam_logger_vlc::player_events, Qt::ConnectionType::QueuedConnection);

        QList<libvlc_event_e> keys = playerHandlers.keys();
        for ( libvlc_event_e&   event : keys )
        {
            m_player->event_activate(event, true);
        }

    }
    return m_player;
}

void      cam_logger_vlc::releasePlayer()
{
    if (m_player)
    {
        m_player->disconnect();
        m_player->stop();

        vlc::vlc_media* media = m_player->set_media(nullptr);

        if (media)
            media->deleteLater();


        m_player->deleteLater();
        m_player = nullptr;
    }

}

void cam_logger_vlc::playChecker()
{
    libvlc_media_stats_t stats =  m_player->get_media_stats();
    if (m_displayedPictures != stats.i_displayed_pictures)
    {
        m_displayedPictures = stats.i_displayed_pictures;
        emit framesChanged(m_displayedPictures);
        playWatchdog.start();
        return;
    }
    if (isStreaming())
    {

    }
    else
    {
        emit onError();
    }

}

void cam_logger_vlc::startPlayWatchDog()
{
    m_displayedPictures = 0;
    playWatchdog.setSingleShot(true);
    playWatchdog.setInterval(10000);
    playWatchdog.start();
}


bool cam_logger_vlc::togglePlaying()
{
    if (m_player)
    {
        return m_player->is_playing() ? m_player->stop() : m_player->play();
    }
    return false;
}
