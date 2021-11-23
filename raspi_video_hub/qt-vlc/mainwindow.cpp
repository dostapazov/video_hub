//
#include <QDebug>
#include <QDir>
//#include <QAction>
#include <QKeyEvent>
#include <stdio.h>
#include "appconfig.h"
#include "mainwindow.h"
#include <QOpenGLWidget>

#include "applog.h"
#include <qpalette.h>
#include <signal.h>
#include <unistd.h>

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
    blinker.start    ();
}


MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent)

{
    setupUi(this);
    devId = appConfig::get_devid();
    connect(this, &MainWindow::cam_switch, this, &MainWindow::onCamSwitch, Qt::ConnectionType::QueuedConnection);

    initStartLoggers();
    init_gpio  ();
    initBlinker();
    init_libvlc      ();
    init_uart        ();
    load_config      ();
    initCamMonitor();
    startCamMonitor();
    startLoggers();

#if defined (DESKTOP_DEBUG_BUILD)
    show();
#else
    showFullScreen();
#endif

}

MainWindow::~MainWindow()
{
}

void MainWindow::initStartLoggers()
{
    connect(&starLoggersTimer, &QTimer::timeout, this, &MainWindow::startLoggers);
    starLoggersTimer.setSingleShot(true);
    starLoggersTimer.setInterval(1000);
}

void MainWindow::init_libvlc()
{
    int argc = sizeof(vlcArgs) / sizeof(vlcArgs[0]);
    vlc::vlc_instance::get_instance(argc, vlcArgs);
    QString vlc_ver = vlc::vlc_instance::get_version();
    appLog::write(LOG_LEVEL_ALWAYS, QString("VLC-lib version ") + vlc_ver);
}



void MainWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);
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
    deinit_all();
    qApp->quit();
}

QList<cam_params_t> MainWindow::readCameraList()
{
    QList<cam_params_t> cams;
    cam_params_t cam_param;
    QStringList camList = appConfig::get_cam_list();
    std::sort(camList.begin(), camList.end());
    foreach (QString camName, camList)
    {

        cam_param.mrl  = appConfig::get_cam_mrl(camName);
        cam_param.id  = appConfig::get_cam_id(camName);
        cam_param.name = appConfig::get_cam_name(camName);
        cam_param.disabled = appConfig::get_cam_logdisabled(camName);


        if (!cam_param.mrl.isEmpty())
        {
            cams.append(cam_param);
        }
    }

#ifdef DESKTOP_DEBUG_BUILD
    if (!cams.count())
    {
        cams.append({1, tr("IPCam100"), tr("rtsp://192.168.0.100:554/media/video1"), false});
        cams.append({1, tr("IPCam101"), tr("rtsp://192.168.0.101:554/media/video1"), false});
    }
#endif


    return cams;
}

void MainWindow::load_config()
{
    QList<cam_params_t> cams = readCameraList();
    for ( cam_params_t& cp : cams)
    {
        loggers.append(new cam_logger(cp));
        QString str = tr("append camera ID=%1 %2 %3").arg(cp.id).arg(cp.name).arg(cp.mrl);
        appLog::write(LOG_LEVEL_DEBUG, str);
    }

#ifdef DESKTOP_DEBUG_BUILD
    appConfig::setValue("VLOG/Folder", "streaming");
#ifdef Q_OS_LINUX
    appConfig::setValue("VLOG/MountPoint", "/home/dostapazov/raspi_log/");
#else
    appConfig::setValue("VLOG/MountPoint", "d:/raspi_log/");
#endif

#endif
}

void MainWindow::deinitLoggers()
{
    foreach (cam_logger* cl, this->loggers)
    {
        cl->stop();
        cl->deleteLater();
    }
    loggers.clear();

}

void MainWindow::deinitMonitor()
{
    cam_monitor->stop();
    delete cam_monitor;
    cam_monitor = nullptr;
}

void MainWindow::deinitFileDeleter()
{
    if (file_deleter)
    {
        file_deleter->requestInterruption();
        file_deleter->wait(2000);
        file_deleter->deleteLater();
        file_deleter = Q_NULLPTR;
    }
}

void MainWindow::deinit_all()
{
    qDebug() << Q_FUNC_INFO << " begin";

    blinker.stop ();
    disconnect(&blinker );
    deinitLoggers();
    deinitMonitor();
    deinitFileDeleter();

    deinitUART   ();
    //appLog::write(2,QString(Q_FUNC_INFO));
    qDebug() << Q_FUNC_INFO << " end";
}

static void addFolder(QString& path, const QString& folder)
{
    QChar slash('/');
    if (!path.endsWith(slash))
        path += slash;

    path += folder.startsWith(slash) ? folder.mid(1) : folder;

    if (!path.endsWith(slash))
        path += slash;
}

bool MainWindow::check_media_drive()
{
    QString strPath =  appConfig::get_mount_point();
    addFolder(strPath, whoami());
    addFolder(strPath, appConfig::get_vlog_folder());
    //appLog::write(0,  tr("check existing streamig dir %1").arg(strPath));

    if ( QDir(strPath).exists() )
    {

        appLog::write(LOG_LEVEL_DEBUG, tr("Begin logging video"));
        m_vlog_root = strPath;
    }
    else
    {
        m_vlog_root.clear();
    }
    bool logEnabled = !m_vlog_root.isEmpty();
    appConfig::set_log_enabled(logEnabled);
    return logEnabled;
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
    blinker.start();
}

void MainWindow::onCamSwitch(quint8 camId)
{
    if (appState.camId != camId)
    {
        appState.camId = camId;
        appConfig::set_mon_camera(camId);
        const cam_logger* clogger = loggers.at(camId);
        QString str = tr("Monitor switch to camera (%1) %2").arg(int(camId)).arg(clogger->get_mrl());
        appLog::write(LOG_LEVEL_CAM_MON, str);
        cam_monitor->startMonitoring(clogger->get_mrl());
        str = QString("Wait data from %1").arg(clogger->get_name());
        label->setText(str);
        activateSelf();
    }
    return;
}



void MainWindow::initCamMonitor()
{
    if (!cam_monitor)
    {
        cam_monitor = new cam_logger({-1, "Monitor logger", ""});
        connect(cam_monitor, &cam_logger::onPlayStart, this, &MainWindow::onStartMon);
        connect(cam_monitor, &cam_logger::onPlayStop, this, &MainWindow::onStopMon);
        connect(cam_monitor, &cam_logger::onError, this, &MainWindow::onMonitorError);
        connect(cam_monitor, &cam_logger::framesChanged, this, &MainWindow::onFramesChanged);
        connect(&switchTimer, &QTimer::timeout, this, &MainWindow::onSwitchTimer);
    }
}

void MainWindow::startCamMonitor()
{
    appLog::write(LOG_LEVEL_CAM_MON, "start_cam_monitor ");
    //m_camWindow = new QOpenGLWidget;
    //m_camWindow = new QWidget;
    switchTimer.start(30000);

    int cam_id = std::max(appConfig::get_mon_camera(), 0);
    emit cam_switch(static_cast<quint8>(cam_id));
}

void MainWindow::onFramesChanged(int frames)
{
    this->FrameNo->setText(QString::number(frames));
}


void MainWindow::onStartMon()
{
    const cam_logger* clogger = loggers.at(appState.camId);
    QString str = QString("%1 is monitored").arg(clogger->get_name());
    label->setText(str);
    FrameNo->setText("-");
    appLog::write(LOG_LEVEL_CAM_MON, str );
#if defined DESKTOP_DEBUG_BUILD
    showMinimized();
#else
    hide();
#endif
    //cam_monitor->getPlayer()->set_fullscreen(true);
}

void MainWindow::onStopMon()
{
    const cam_logger* clogger = loggers.at(appState.camId);
    QString str = QString("%1 lost connection").arg(clogger->get_name());
    appLog::write(LOG_LEVEL_CAM_MON, str);
    label->setText(str);
    activateSelf();
}

void MainWindow::activateSelf()
{

    showNormal();
#if !defined DESKTOP_DEBUG_BUILD
    showFullScreen();
# endif
    activateWindow();

}

void MainWindow::onMonitorError()
{
    const cam_logger* clogger = loggers.at(appState.camId);
    QString str = QString("Camera %1 not respond").arg(clogger->get_name());
    if (isVisible())
        appLog::write(LOG_LEVEL_CAM_MON, str);
    onStopMon();
    cam_monitor->startMonitoring( clogger->get_mrl());
}

void MainWindow::startLoggers()
{
    starLoggersTimer.stop();
    if (m_vlog_root.isEmpty() && check_media_drive())
    {
        start_file_deleter();
        int timeDuration = appConfig::get_time_duration();

        foreach (cam_logger* cl, loggers)
        {
            cl->startStreaming(m_vlog_root, timeDuration);
        }
    }
    else
        starLoggersTimer.start();
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
        appLog::write(LOG_LEVEL_ALWAYS, "update command present") ;
        upd_cmd_file.remove();
        if (do_rename_recorder())
        {
            appLog::write(LOG_LEVEL_ALWAYS, "vhub updated success ... go to reboot") ;
            system("sudo reboot");
        }
        else
            appLog::write(LOG_LEVEL_ALWAYS, "vhub updated error. what can i do?") ;
    }
}

#ifdef DESKTOP_DEBUG_BUILD

static quint8 getNextCamId(int count, int current, bool increment)
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
            cam_monitor->togglePlaying();
        default:
            break;
    }
}
#endif

void MainWindow::onSwitchTimer()
{
    //emit cam_switch(appState.camId ? 0 : 1);
}

