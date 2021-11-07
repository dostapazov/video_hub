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

    playerWatchDog.setInterval(PLAYER_RESPONSE_TIMEOUT);
    connect(&playerWatchDog, &QTimer::timeout, this, &cam_logger_vlc::playerHungDetected, Qt::ConnectionType::QueuedConnection);
    initPlayerHandlers();
}

cam_logger_vlc::~cam_logger_vlc()
{
    stopStreaming();
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
    int interval = 1 * 60 * 1000;// 1 минуа
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



void     cam_logger_vlc::startStreaming(const QString folder, int timeDuration)
{
    mStorageFolder = folder;
    m_time_duration = timeDuration ;
    nextFile();
}

void     cam_logger_vlc::stopStreaming  ()
{
    releasePlayer();
}



void cam_logger_vlc::setupMediaForStreaming()
{
    QString str;
    QDateTime dtm     = QDateTime::currentDateTime();
    m_file_timelen    = get_time_interval(dtm);
    if (m_file_timelen)
    {
        QString file_name = get_file_name(dtm);

        m_next_media->add_option(":no-audio");
        m_next_media->add_option(":no-overlay");
        m_next_media->add_option(":sout-mp4-faststart");

        str = tr(":network-caching=%1").arg(m_network_caching);
        m_next_media->add_option(str.toLocal8Bit().constData());


        str = tr(":sout=#standard{access=file, mux=ts,dst=%1}").arg(file_name);
        m_next_media->add_option(str.toLocal8Bit().constData());
        m_next_media->add_option(":demux=h264");
    }
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
    QString str;

    m_file_timelen = 0;
    m_next_media   = new vlc::vlc_media;
    if (m_next_media)
    {
        if (m_next_media->open_location(get_mrl().toLocal8Bit().constData()))
        {

            m_next_media->add_option(":rtsp-timeout=5000");
            setupMediaForStreaming();
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
    player_event_handler_t handler = playerHandlers.value(static_cast<libvlc_event_e>(event.type));
    if (handler)
        handler(m_player);
}

void cam_logger_vlc::nextFile()
{
    if (cutTimer.isActive())
        cutTimer.stop ();

    createPlayer();

    vlc::vlc_media* old_media = m_player->set_media(get_next_media());
    m_player->play();
    if (old_media)
        old_media->deleteLater();
}


void cam_logger_vlc::initPlayerHandlers()
{
    namespace p = std::placeholders;
    playerHandlers[libvlc_MediaPlayerPlaying] = std::bind(&cam_logger_vlc::OnPlayerPlaying, this, p::_1);
    playerHandlers[libvlc_MediaPlayerStopped] = std::bind(&cam_logger_vlc::OnPlayerStopped, this, p::_1);
    playerHandlers[libvlc_MediaPlayerEncounteredError] = std::bind(&cam_logger_vlc::OnPlayerError, this, p::_1);
    playerHandlers[libvlc_MediaPlayerEndReached] = std::bind(&cam_logger_vlc::OnPlayerEndReached, this, p::_1);
    playerHandlers[libvlc_MediaPlayerPositionChanged] = std::bind(&cam_logger_vlc::OnPlayerPosition, this, p::_1);
}

void cam_logger_vlc::OnPlayerStopped(vlc::vlc_player* player)
{
    Q_UNUSED(player)
    QString str = tr("%1 player stopped,replace media").arg(get_name());
    appLog::write(0, str);
    qDebug() << str;
    m_player->set_media(get_next_media())->deleteLater();
    m_player->play();
}

void cam_logger_vlc::OnPlayerPlaying(vlc::vlc_player* player)
{
    Q_UNUSED(player)
    QString str = tr("%1 player playing").arg(get_name());
    appLog::write(0, str);
    cutTimer.setInterval(m_file_timelen);
    cutTimer.start();

}

void cam_logger_vlc::OnPlayerError(vlc::vlc_player* player)
{
    Q_UNUSED(player)
    QString str = tr("%1 player stopped, errors encountered %2").arg(get_name()).arg(m_player->get_last_errors().join(", "));
    appLog::write(0, str);
    qDebug() << str;

}


void cam_logger_vlc::OnPlayerEndReached(vlc::vlc_player* player)
{
    Q_UNUSED(player)
    QString str = tr("%1 player end reached").arg(get_name());
    appLog::write(0, str);
    qDebug() << str;
    create_next_media();
}


void cam_logger_vlc::OnPlayerPosition(vlc::vlc_player* player)
{
    Q_UNUSED(player)
    QString str = tr("%1 player position changed").arg(get_name());
    playerWatchDog.stop();
    playerWatchDog.start();
    //qDebug() << str;
}



void      cam_logger_vlc::createPlayer()
{
    if (!m_player)
    {
        m_player = new vlc::vlc_player;
        for ( auto event : playerHandlers.keys() )
        {
            m_player->event_activate(event, true);
        }
        connect(m_player, &vlc::vlc_player::player_event, this, &cam_logger_vlc::player_events, Qt::ConnectionType::QueuedConnection);
    }
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

void     cam_logger_vlc::playerHungDetected()
{
    qDebug() << "Player hung detected!!!";
    Упростить создание vcl_media
    Пересоздать player при обрыве связи
    удалять файлы нулевой длины
}


