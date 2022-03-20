/***********************************************
 * class to save video files
 * from media stream with defined time lenght
 * via libvlc (!working with VLC 3.0.4 !)
 * implementation
 *
 * OStapenko D. V. 2018-11-07 NIKTES
 *
 * *********************************************/

#include "cam_logger.h"
#include "qdatetime.h"
#include <qdir.h>
#include "applog.h"
#include "appconfig.h"
#include <QFile>

cam_logger::cam_logger(const cam_params_t& aParams, QObject* parent )
    : QObject(parent)
{
    m_params = aParams;
    cutTimer.setSingleShot(true);
    connect(&cutTimer, &QTimer::timeout, this, &cam_logger::nextFile, Qt::ConnectionType::QueuedConnection);
    connect(&playWatchdog, &QTimer::timeout, this, &cam_logger::playChecker, Qt::ConnectionType::QueuedConnection);
    initPlayerHandlers();
}

cam_logger::~cam_logger()
{
    stop();
    QThread::msleep(200);
}

bool cam_logger::isEventSupport()
{
    constexpr int VLC_VER_EVENT_WORK = 3;
    QString     str  = vlc::vlc_instance::get_version();
    QStringList sl = str.split(".");
    return  (sl.size() > 1 && sl.at(0).toInt() >= VLC_VER_EVENT_WORK) ? true : false;
}

constexpr long SEC_MSECS  = 1000;
constexpr long MIN_MSECS  = 60 * SEC_MSECS;
constexpr long HOUR_MSECS = 60 * MIN_MSECS;

void limitDuration(const QDateTime& dtm, long& duration)
{
    QDateTime endDtm = dtm.addMSecs(duration);
    if (endDtm.time().second() || endDtm.time().msec())
    {
        duration -= endDtm.time().msec();
        duration -= endDtm.time().second() * SEC_MSECS;
        endDtm = dtm.addMSecs(duration);
    }

    if (endDtm.date().day() != dtm.date().day())
    {

        duration -= endDtm.time().minute() * MIN_MSECS;
        duration -= endDtm.time().hour() * HOUR_MSECS;
    }
}

int    cam_logger::get_time_interval(const QDateTime& dtm)
{
    int time_duration = appConfig::get_time_duration();
    QTime time = dtm.time();
    long duration_ms = time_duration * MIN_MSECS;
    long current_ms  = time.hour() * HOUR_MSECS + time.minute() * MIN_MSECS + time.second() * SEC_MSECS;
    ldiv_t ldt = ldiv(current_ms, duration_ms);
    duration_ms -= ldt.rem;
    limitDuration(dtm, duration_ms);

    return duration_ms;
}

QString     cam_logger::get_file_name(const QDateTime& dtm)
{
    QString spath    = QString("%1/%2/%3")
                       .arg(m_StorageFolder)
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
    while (!QFile::exists(file_name) && counter < 100);

    return file_name;
}

bool cam_logger::setMonitorWidget( QWidget* widget)
{
    WId id = widget ? widget->winId() : 0;
    if ( id != WId(m_logger_player->get_drawable()))
    {
#ifdef Q_OS_WIN
        m_logger_player->set_drawable(HWND(id));
#else
        m_logger_player->set_drawable(id);
#endif

    }
    return true;
}

bool cam_logger::startMonitoring( const QString& mrl)
{
    qInfo() << "startMonitoring " << m_logger_player;
    m_StreamingMode = false;
    if ( mrl.isEmpty())
        return false;

    //releasePlayer();
    createPlayer();
    m_params.mrl = mrl;
    vlc::vlc_media* media = create_media();
    media = m_logger_player->set_media(media);
    m_logger_player->play();
    if (media)
    {
        media->close();
        media->deleteLater();
    }

    startPlayWatchDog();
    return true;
}

bool cam_logger::startStreaming(const QString& folder)
{
    if(this->m_params.disabled)
        return false;

    m_StreamingMode = true;
    m_StorageFolder = folder;
    connect(this, &cam_logger::onError, this, &cam_logger::nextFile);
    nextFile();
    return true;
}

void     cam_logger::stop  ()
{
    releasePlayer();
}

int cam_logger::setupMediaForStreaming(vlc::vlc_media* media)
{
    QString str;
    QDateTime dtm     = QDateTime::currentDateTime();
    QString fileName = get_file_name(dtm);
    m_streamFiles.append(fileName);
    int time_len      = get_time_interval(dtm);
    media->add_option(":no-overlay");
    media->add_option(":rtsp-timeout=5000");
    media->add_option(":sout-mp4-faststart");

    str = QString(":network-caching=%1").arg(m_network_caching);
    media->add_option(str.toLocal8Bit().constData());

    str = QString(":sout=#standard{access=file, mux=ts,dst=%1}").arg(fileName);
    media->add_option(str.toLocal8Bit().constData());
    media->add_option(":demux=h264");

    return time_len + m_network_caching;
}

void cam_logger::set_mrl(const QString& mrl)
{
    if (m_params.mrl != mrl)
    {
        m_params.mrl = mrl;
        if (m_logger_player && m_logger_player->hasMedia())
        {
            m_logger_player->open_mrl(mrl);
        }
    }
}

vlc::vlc_media*  cam_logger::create_media()
{
    QString str;
    m_file_timelen = 0;
    vlc::vlc_media* media   = new vlc::vlc_media;
    if (media)
    {

        if (media->open_location(get_mrl().toLocal8Bit().constData()))
        {
            media->add_option(":no-audio");
            if (isStreaming())
            {
                str = QString("%1 create next media ").arg(get_name());
                m_file_timelen = setupMediaForStreaming(media);
                div_t t     = div(m_file_timelen, 1000);
                str += QString(" interval %1. %2").arg(t.quot).arg(t.rem);
                appLog::write(LOG_LEVEL_VLC, str);
            }
            else
                media->add_option(":rtsp-timeout=20000");
        }
        else
        {
            str = QString("%1 error open  ").arg(get_name()).arg(get_mrl());
            appLog::write(LOG_LEVEL_VLC, str);
        }
    }
    return media;
}

void cam_logger::removeEmptyPreviousFile()
{
    while (m_streamFiles.count() > 2)
    {
        QFile file;
        file.setFileName(m_streamFiles.takeFirst());
        if (file.exists() && 0 == QFileInfo(file).size())
        {
            //qDebug().noquote() << "remove empty file " << file.fileName();
            file.remove();
        }
    }
}

void cam_logger::nextFile()
{
    if (cutTimer.isActive())
        cutTimer.stop ();

    createPlayer();
    vlc::vlc_media* media = create_media();
    media = m_logger_player->set_media(media);
    m_Playing = false;
    m_logger_player->play();

    if (media)
    {
        media->close();
        media->deleteLater();
    }

    removeEmptyPreviousFile();
    startPlayWatchDog();
}


void cam_logger::initPlayerHandlers()
{
    namespace p = std::placeholders;
    playerHandlers[libvlc_MediaPlayerPlaying] = std::bind(&cam_logger::OnPlayerPlaying, this, p::_1);
    playerHandlers[libvlc_MediaPlayerStopped] = std::bind(&cam_logger::OnPlayerStopped, this, p::_1);
}

void cam_logger::OnPlayerStopped(vlc::vlc_player* player)
{
    Q_UNUSED(player)
    QString str = QString("%1 stopped").arg(get_name());
    appLog::write(LOG_LEVEL_VLC, str);
    m_Playing = false;
    emit onPlayStop();
}

void cam_logger::OnPlayerPlaying(vlc::vlc_player* player)
{
    Q_UNUSED(player)
    QString str = QString("%1 playing").arg(get_name());
    appLog::write(LOG_LEVEL_VLC, str);

    if (isStreaming())
    {
        m_logger_player->set_drawable(0);
        cutTimer.stop();
        cutTimer.setInterval(m_file_timelen);
        cutTimer.start();
    }
    startPlayWatchDog();
    emit onPlayStart();

}

void   cam_logger::player_events(const libvlc_event_t event)
{
    player_event_handler_t handler = playerHandlers.value(static_cast<libvlc_event_e>(event.type));
    if (handler)
        handler(m_logger_player);
}

vlc::vlc_player*   cam_logger::createPlayer()
{
    if (!m_logger_player)
    {
        m_logger_player = new vlc::vlc_player;
        connect(m_logger_player, &vlc::vlc_player::player_event, this, &cam_logger::player_events, Qt::ConnectionType::QueuedConnection);

        QList<libvlc_event_e> keys = playerHandlers.keys();
        for ( libvlc_event_e&   event : keys )
        {
            m_logger_player->event_activate(event, true);
        }

    }
    return m_logger_player;
}

void      cam_logger::releasePlayer()
{
    if (m_logger_player)
    {
        m_logger_player->disconnect();
        m_logger_player->stop();

        vlc::vlc_media* media = m_logger_player->set_media(nullptr);

        if (media)
        {
            media->close();
            media->deleteLater();
        }

        m_logger_player->deleteLater();
        m_logger_player = nullptr;
    }
}

void cam_logger::playChecker()
{

    libvlc_media_stats_t stats =  m_logger_player->get_media_stats();
    if (m_demuxReadBytes != stats.i_demux_read_bytes)
    {
        if (!isStreaming())
        {
            emit framesChanged(stats.i_displayed_pictures, stats.i_lost_pictures);
        }
        m_demuxReadBytes = stats.i_demux_read_bytes;
        playWatchdog.start();
        m_Playing = stats.i_demux_read_bytes;
        return;
    }

    if (m_Playing)
    {
        appLog::write(LOG_LEVEL_VLC, QString("%1 not respond").arg(get_name()));

    }

    emit onError();
}


void cam_logger::startPlayWatchDog()
{
    m_demuxReadBytes = -1;
    if (playWatchdog.isActive())
        playWatchdog.stop();
    playWatchdog.setSingleShot(true);
    playWatchdog.setInterval(PLAY_WATCHDOG_TIMEOUT);
    playChecker();
}

bool cam_logger::togglePlaying()
{
    if (m_logger_player)
    {
        return m_logger_player->is_playing() ? m_logger_player->stop() : m_logger_player->play();
    }
    return false;
}

uint8_t cam_logger::getErrorBit()
{
    if(m_Playing || 0 == m_params.id)
        return 0;

    return 1<<(m_params.id-1);
}
