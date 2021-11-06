//
#include <QEventLoop>
#include <QDateTime>
#include <QDebug>
#include <QDir>
//#include <QAction>
#include <QKeyEvent>
#include <stdio.h>
#include "appconfig.h"
#include "mainwindow.h"
#include "applog.h"
#include <qpalette.h>
#include <signal.h>
#include <unistd.h>

#ifndef DESKTOP_DEBUG_BUILD
    #include <wiringPi.h>
#else
    #define digitalRead(x)
    #define digitalWrite(x,y)

#endif


const char* const MainWindow::vlcArgs[] =
{
    "--noaudio",
    "--no-overlay",
    "--network-caching=300",
#ifdef DESKTOP_DEBUG_BUILD
    "--verbose=-1",
#else
    "--verbose=-1",
#endif

};


void MainWindow::initBlinker()
{
#ifdef QT_DEBUG
    blinker.setInterval(1000);
#else
    blinker.setInterval(333);
#endif

    connect(&blinker, &QTimer::timeout, this, &MainWindow::on_blink, Qt::ConnectionType::QueuedConnection);
}

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent)
{
    setupUi(this);
    appState = PCK_STATE_t {0xFF, 77, 777};
    connect(this, &MainWindow::cam_switch, this, &MainWindow::on_cam_switch, Qt::ConnectionType::QueuedConnection);
    init_gpio  ();
    initBlinker();

    blinker.start    ();
    init_libvlc      ();
    init_uart        ();
    load_config      ();

}

MainWindow::~MainWindow()
{
    deinit_all();
}

void MainWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);
    start_cam_monitor();
}

void MainWindow::init_libvlc()
{
    int argc = sizeof(vlcArgs) / sizeof(vlcArgs[0]);
    vlc::vlc_instance::get_instance(argc, vlcArgs);
    QString vlc_ver = vlc::vlc_instance::get_version();
    appLog::write(0, vlc_ver);
}



void MainWindow::start_file_deleter()
{
    if (file_deleter && !file_deleter->isRunning())
    {
        file_deleter->requestInterruption();
        file_deleter->wait(2000);
        file_deleter->deleteLater();
        file_deleter = Q_NULLPTR;
    }

    if (!file_deleter && !m_vlog_root.isEmpty())
    {
        file_deleter = new FileDeleterThread(this);
        file_deleter->set_stream_root(m_vlog_root);
        int v = appConfig::get_disc_free_space_percent() ;

        file_deleter->set_keep_free_percent(v);
        file_deleter->start(QThread::IdlePriority);
    }
}



void MainWindow::closeEvent(QCloseEvent* event)
{
    Q_UNUSED(event);
    QMainWindow::closeEvent(event);
    qApp->quit();
}

QList<cam_params_t> MainWindow::readCameraList()
{
    QList<cam_params_t> cams;
#ifdef DESKTOP_DEBUG_BUILD
    //cams.append({1, tr("IPCam100"), tr("rtsp://192.168.0.100:554/media/video1"), false});
    cams.append({1, tr("IPCam101"), tr("rtsp://192.168.0.101:554/media/video1"), false});
#else
    QStringList camList = appConfig::get_cam_list();
    std::sort(camList.begin(), camList.end());
    foreach (QString camName, camList)
    {
        cam_params_t cam_param;
        cam_param.mrl  = appConfig::get_cam_mrl(camName);
        cam_param.id  = appConfig::get_cam_id(camName);
        cam_param.name = appConfig::get_cam_name(camName);
        cam_param.disabled = appConfig::get_cam_logdisabled(camName);


        if (!cam_param.mrl.isEmpty())
        {
            cams.append(cam_param);
        }
    }
#endif
    return cams;
}

void MainWindow::load_config()
{

    QList<cam_params_t> cams = readCameraList();

    for ( cam_params_t& cp : cams)
    {
        loggers.append(new cam_logger_vlc(cp));
        QString str = tr("append camera ID=%1 %2 %3").arg(cp.id).arg(cp.name).arg(cp.mrl);
        appLog::write(0, str);
    }

}


void MainWindow::deinit_all()
{
    qDebug() << Q_FUNC_INFO << " begin";

    blinker.stop ();
    disconnect(&blinker );

    foreach (cam_logger_vlc* cl, this->loggers)
    {
        cl->stop_streaming();
        delete cl;
    }

    deinit_player();

    if (file_deleter)
    {
        file_deleter->requestInterruption();
        file_deleter->wait(2000);
        file_deleter->deleteLater();
        file_deleter = Q_NULLPTR;
    }


    deinitUART   ();
    //appLog::write(2,QString(Q_FUNC_INFO));
    qDebug() << Q_FUNC_INFO << " end";
}


bool MainWindow::check_media_drive( )
{
    QString strPath;
    m_vlog_root.clear();
    strPath = tr("%1/%2/%3")
              .arg(appConfig::value("VLOG/MountPoint").toString())
              .arg(whoami())
              .arg(appConfig::value("VLOG/Folder").toString());
    QString str = tr("check existing dir %1").arg(strPath);
    appLog::write(0, str);
    if ( QDir(strPath).exists() )
    {
        m_vlog_root = strPath;
        appLog::write(0, tr("directory exists"));
        return true;
    }
    appLog::write(0, tr("directory NOT exist: write videolog disabled "));
    return  false;
}


QString MainWindow::whoami()
{
    QString res = qgetenv("USER");
    if (res.isEmpty())
        res = qgetenv("USERNAME");
    return res;
}


void MainWindow::on_blink()
{
    blinker.stop();
    led_state   = !led_state;
    digitalWrite(PIN_LED1, led_state);
    readCPUtemper();
    check_need_update();
    //check player state
    blinker.start();
}

void MainWindow::createPlayer()
{
    if (!m_mon_player)
    {
        appLog::write(0, "create new mon_player");
        m_mon_player  =  new vlc::vlc_player;
        connect(m_mon_player, &vlc::vlc_player::player_event, this, &MainWindow::mon_player_events, Qt::ConnectionType::QueuedConnection);
        m_mon_player->event_activate(libvlc_event_e::libvlc_MediaPlayerStopped, true);
        m_mon_player->event_activate(libvlc_event_e::libvlc_MediaPlayerPlaying, true);
        m_mon_player->event_activate(libvlc_event_e::libvlc_MediaPlayerEncounteredError, true);
        m_mon_player->event_activate(libvlc_event_e::libvlc_MediaPlayerPositionChanged, true);
    }
}

void MainWindow::on_cam_switch(quint8 cam_num)
{
    QObject* sobj = sender();
    qDebug() << Q_FUNC_INFO << cam_num;
    QString str = tr("on_cam_switch(%1) sender %2").arg(int(cam_num)).arg(sobj ? sobj->objectName() : "none");
    qDebug() << str;
    appLog::write(6, str);
    is_cam_online = false;

    if ((appState.camId != cam_num || !m_mon_player || !m_mon_player->has_media()) && cam_num < this->loggers.count() )
    {
        createPlayer();
        const cam_logger_vlc* clogger = loggers.at(cam_num);
        label->setText(tr("wait data from camera %1 ").arg(clogger->get_name()));

        appState.camId = cam_num;
        appConfig::setValue(tr("DEV/CAMERA"), appState.camId);

        vlc::vlc_media* media = new vlc::vlc_media;
        if (media->open_location(clogger->get_mrl().toLocal8Bit().constData()))
        {

            media->add_option(":rtsp-timeout=5000");
            media = m_mon_player->set_media(media);
            if (media)
                media->deleteLater();
            m_mon_player->play();

#if !defined (DESKTOP_DEBUG_BUILD)
            m_mon_player->set_fullscreen(true);
#endif
            moncam_timer.stop ();
            moncam_timer.start(CAMERA_WDT_INTERVAL);

        }
    }
}

void MainWindow::mon_player_events    (const libvlc_event_t event)
{
    vlc::vlc_player* player = const_cast<vlc::vlc_player*>(dynamic_cast<vlc::vlc_player*>(sender()));
    if (player)
    {
        switch (event.type)
        {
            case libvlc_MediaPlayerStopped:
            {
                QString str = tr("Mon player stopped ");
                appLog::write(6, str);
                const cam_logger_vlc* clogger = loggers.at(appState.camId);
                label->setText(tr("%1 lost connection ").arg(clogger->get_name()));

            }
            break;
            case libvlc_MediaPlayerPlaying:
            {
                appLog::write(6, "Mon player playing");
                const cam_logger_vlc* clogger = loggers.at(appState.camId);
                label->setText(tr("%1 working ").arg(clogger->get_name()));
            }
            break;
            case libvlc_MediaPlayerEncounteredError:
            {
                QString str = tr("Mon player errors %1").arg(player->get_last_errors().join(", "));
                appLog::write(0, str);
                if (m_mon_player->has_media())
                    delete m_mon_player->set_media(nullptr);
            }
            break;

            case  libvlc_MediaPlayerPositionChanged:
                qDebug() << "Media player position changed";
                break;

        }
    }
}


void MainWindow::on_moncam_timeout()
{
    //Слежение за работой монитора
    if (m_mon_player)
    {
        if (m_mon_player && m_mon_player->get_state() !=  libvlc_Playing)
        {
            deinit_player();
            const cam_logger_vlc* clogger = loggers.at(appState.camId);
            if (is_cam_online)
                label->setText(tr("Camera %1 disconnected").arg(clogger->get_name()));
            on_cam_switch(appState.camId);
        }
        else
            is_cam_online = true;
    }
}

void MainWindow::on_monloger_timeout()
{
    if (m_vlogger_state == vl_disable)
    {
        m_vlogger_state = check_media_drive() ? vl_enable : vl_disable;
        appConfig::setValue("VLOG/Enabled", m_vlogger_state != vl_disable ? true : false);
        if (m_vlogger_state != vl_disable)
            start_time_sync();
#ifdef QT_DEBUG
        start_loggers();
#endif
    }
}

void MainWindow::start_cam_monitor()
{
    appLog::write(2, "start_cam_monitor next must be start_cam_switch");
    int cam_id = std::min(appConfig::get_mon_camera(), 0);
    emit cam_switch(static_cast<quint8>(cam_id));
    connect(&moncam_timer, &QTimer::timeout, this, &MainWindow::on_moncam_timeout, Qt::ConnectionType::QueuedConnection);
    connect(&monlog_timer, &QTimer::timeout, this, &MainWindow::on_monloger_timeout, Qt::ConnectionType::QueuedConnection);
    start_cam_switch(true);
    monlog_timer.start(VLOG_WDT_INTERVAL);
}


void MainWindow::start_time_sync()
{
//Запускаем синхронизацию времени с камерой
    connect(&cam_time_sync, &CamTimeSync::synchronized, this, &MainWindow::cam_time_synchronized, Qt::ConnectionType::QueuedConnection);
    connect(&cam_time_sync, &CamTimeSync::time_difference, this, &MainWindow::cam_time_difference, Qt::ConnectionType::QueuedConnection);
    cam_time_synchronized(false);
#ifdef QT_DEBUG
    //start_loggers();
#endif

}


void MainWindow::cam_time_synchronized(bool ok)
{

    if (!ok)
    {
        //Ошибка синхронизации времени выбираем другую камеру
        int cams_count = loggers.count();
        if ((++cam_time_synchro) >= cams_count)
            cam_time_synchro = 0;
        QString str = cams_count ? loggers.at(cam_time_synchro)->get_mrl() : QString();
        cam_time_sync.start_sync(str, cams_count ? 3000 : 5000);
        str = tr("Sync time fault try at next cam %1 from %2").arg(cam_time_synchro).arg(cam_time_sync.host());
        qDebug() << str;
#if defined (QT_DEBUG)
        start_loggers();
#endif
    }
    else
    {
        cam_time_sync.schedule_next_request(5000);
        qDebug() << tr("time sync success from %1").arg(cam_time_sync.host());
        start_loggers();
    }

}


void MainWindow::cam_time_difference(const QDateTime& dt, const qint64& diff)
{
    //Рассинхронизация времени с камерой больше порога
    cam_time_sync.schedule_next_request(5000);
    qDebug() << tr("%1 time difference %2").arg(cam_time_sync.host()).arg(diff);
    QString command = tr("sudo date --set=\"%1\"").arg(dt.toString("yyyy-MM-dd hh:mm:ss"));
#ifdef Q_OS_LINUX
    system(command.toLocal8Bit().constData());
#endif

}

void MainWindow::start_loggers()
{

    if (m_vlogger_state != vl_working)
    {
        m_vlog_tmlen = appConfig::value("COMMON/video_length").toInt() * 60;
        if (0 == m_vlog_tmlen)
            m_vlog_tmlen = 3600;

        start_file_deleter();

        foreach (cam_logger_vlc* cl, loggers)
        {
            cl->start_streaming(m_vlog_root, m_vlog_tmlen);
        }
        m_vlogger_state = vl_working;
    }

}

void MainWindow::init_gpio()
{

#ifndef DESKTOP_DEBUG_BUILD
    wiringPiSetup();

    pinMode(PIN_SWITCH, INPUT);
    pullUpDnControl(PIN_SWITCH, PUD_UP);

    pinMode(PIN_LED1, OUTPUT);


    //init fan control gpio pin
    pinMode(PIN_FAN, OUTPUT);
    pullUpDnControl(PIN_FAN, PUD_DOWN);
    appState.fanState = 1;
    digitalWrite(PIN_FAN, 1);
#endif

}

void MainWindow::readCPUtemper()
{

    FILE* thermal;
    int n = 0;
#ifdef Q_OS_LINUX
    const char* temper_file_name = "/sys/class/thermal/thermal_zone0/temp";
#else
    const char* temper_file_name = "d:/temp.txt";
#endif

    int onTemper  = appConfig::value("FAN/StartTemper").toInt() ;
    int offTemper = appConfig::value("FAN/StopTemper" ).toInt() ;
    onTemper  = 1000 * qMax(onTemper, 45);
    offTemper = 1000 * qMax(offTemper, 40);

    thermal = fopen(temper_file_name, "r");

    if (thermal)
    {
        int rv = 0;
        n = fscanf(thermal, "%d", &rv);
        fclose(thermal);
        appState.temper = static_cast<quint16>(rv);

    }

    if ( !thermal || !n )
    {
        appState.temper = static_cast<quint16>(onTemper + 1);
        if (!appState.fanState) // Вентилятор выключен пишем в лог и включаем принудительно
            appLog::write(0, "error get temperature from mon file. switch fan ON");
    }

    //qDebug("current temp %u  fan state %d onTemp %d offTemp %d ",(unsigned int)appState.temper, (int)appState.fanState,onTemper,offTemper);
    bool fan_state_changed = false;
    if ((appState.temper >= onTemper) && (appState.fanState != 1))
    {
        appState.fanState = 1;
        fan_state_changed = true;
        QString str = tr("Temper  %1 > %2, starting fan").arg(appState.temper).arg(onTemper);
        appLog::write(6, str);
    }

    if ((appState.temper <= offTemper) && (appState.fanState != 0))
    {
        appState.fanState = 0;
        fan_state_changed = true;
        QString str = tr("Temper  %1 < %2, stopping fan").arg(appState.temper).arg(onTemper);
        appLog::write(6, str);
    }

    if (fan_state_changed)
    {
        digitalWrite(PIN_FAN, appState.fanState);
    }
}


void MainWindow::start_cam_switch(bool enable)
{
    QString str = tr("start cam switch timer %1").arg(enable);
    appLog::write(0, str);
    if (enable)
        moncam_timer.start(CAMERA_WDT_INTERVAL);
    else
        moncam_timer.stop();

}

void MainWindow::deinit_player()
{
    qDebug() << Q_FUNC_INFO << " begin";
    if (m_mon_player)
    {
        m_mon_player->disconnect();
        m_mon_player->stop();

        vlc::vlc_media* media = m_mon_player->set_media(nullptr);
        if (media)
            media->deleteLater();


        m_mon_player->deleteLater();
        m_mon_player = nullptr;
    }


    qDebug() << Q_FUNC_INFO << " end";
}

void MainWindow::on_bTestUpdate_clicked()
{
    QCoreApplication::exit(UPDATE_EXIT_CODE);
}

QString MainWindow::get_update_file_name()
{
    QString upd_file = tr("%1.update").arg(QCoreApplication::applicationFilePath());
    return upd_file;
}

bool MainWindow::do_rename_recorder()
{
    bool ret = false;
    QString app = QCoreApplication::applicationFilePath();
    QString   app_upd = get_update_file_name();

    if (QFile(app_upd).exists())
    {
        ret =
            (0 == unlink(app.toLocal8Bit().constData()) &&
             0 == rename(app_upd.toLocal8Bit().constData(), app.toLocal8Bit().constData())
            );
    }
    return ret;
}

void MainWindow::check_need_update()
{
    QString upd_cmd =  QCoreApplication::applicationDirPath();
    upd_cmd += tr("/.apply_update");
    QFile upd_cmd_file(upd_cmd);
    if (upd_cmd_file.exists())
    {
        appLog::write(0, "update command present") ;
        upd_cmd_file.remove();
        if (do_rename_recorder())
        {
            appLog::write(0, "vhub updated success ... go to reboot") ;
            system("sudo reboot");
        }
        else
            appLog::write(0, "vhub updated error. what can i do?") ;
    }
}

quint8 getNextCamId(int count, int current, bool increment)
{
    current += increment ? 1 : -1;
    if (current < 0 )
        return count - 1;

    if (current >= count)
        return 0;

    return current;
}

void MainWindow::keyReleaseEvent(QKeyEvent* event)
{
    QMainWindow::keyReleaseEvent(event);
#ifdef DESKTOP_DEBUG_BUILD
    switch (event->key())
    {
        case Qt::Key_Left:
        case Qt::Key_Right:
        {
            quint8 camId =  getNextCamId(loggers.size(), appState.camId, event->key() == Qt::Key_Right);
            emit cam_switch(camId);
        }

        break;
        case Qt::Key_Space:
            m_mon_player->is_playing() ?  m_mon_player->stop() : m_mon_player->play();
            break;
        default:
            break;
    }
#endif
}

