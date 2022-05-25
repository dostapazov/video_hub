//
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
    LabelVersion->setText(version());
    devId = appConfig::get_devid();
    connect(this, &MainWindow::cam_switch, this, &MainWindow::onCamSwitch, Qt::ConnectionType::QueuedConnection);
    appState.camId = -1;
    appState.fanState  = FAN_OFF;
    appState.temper = 0;

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
    if (appState.camId != camId )
    {
        if ( camId < loggers.count())
        {

            appState.camId = camId;
            appConfig::set_mon_camera(camId);
            const cam_logger* clogger = loggers.at(camId);
            QString str = tr("Switch to %1 [%2]")
                          .arg(clogger->get_name())
                          .arg(clogger->get_mrl());

            appLog::write(LOG_LEVEL_CAM_MON, str);
            cam_monitor->startMonitoring(clogger->get_mrl());
            cam_monitor->setMonitorWidget(m_CamWidget);
            str = QString("%1 wait data from %2")
                  .arg(clogger->get_name())
                  .arg(QDateTime::currentDateTime().toString("dd-MM-yy hh:mm:ss"))
                  ;
            label->setText(str);
            m_FramesDisplayed = m_FramesLost = 0;
            activateSelf();
        }
        else
        {
            appLog::write(LOG_LEVEL_DEBUG,
                          QString("selected camera Id[%1] is grow than camera count [%2]")
                          .arg(quint32(camId))
                          .arg(loggers.count())
                         );
        }

    }
}

void MainWindow::initCamMonitor()
{
    if (!cam_monitor)
    {
        cam_monitor = new cam_logger({-1, "Monitor logger", ""});
        connect(cam_monitor, &cam_logger::onPlayStart, this, &MainWindow::onStartMon);
        connect(cam_monitor, &cam_logger::onPlayStop, this, &MainWindow::onMonitorError);
        connect(cam_monitor, &cam_logger::onError, this, &MainWindow::onMonitorError);
        connect(cam_monitor, &cam_logger::framesChanged, this, &MainWindow::onFramesChanged);
        m_CamWidget = new QWidget;
    }
}

void MainWindow::startCamMonitor()
{
    appLog::write(LOG_LEVEL_CAM_MON, "start_cam_monitor ");
    int cam_id = std::max(appConfig::get_mon_camera(), 0);
    emit cam_switch(static_cast<quint8>(cam_id));
}

void MainWindow::onFramesChanged(int displayFrames, int lostFrames)
{
    m_FramesDisplayed += displayFrames;
    m_FramesLost += lostFrames;
    FrameNo->setText(QString::number(m_FramesDisplayed));
    LostFrames->setText(QString::number(m_FramesLost));
}


bool  MainWindow::isCamMonitorActive()
{
    return  m_CamWidget->isVisible() && m_CamWidget->isActiveWindow();
}

void MainWindow::activateCamMonitor()
{
    if (m_CamWidget)
    {
        m_CamWidget->showFullScreen();
        m_CamWidget->activateWindow();
        m_CamWidget->setCursor(QCursor(Qt::BlankCursor));
    }
}

void MainWindow::activateSelf()
{
    showFullScreen();
    activateWindow();
}

void MainWindow::onStartMon()
{
    if (!isCamMonitorActive())
    {
        const cam_logger* clogger = loggers.at(appState.camId);
        QString str = QString("%1 is monitored from %2")
                      .arg(clogger->get_name())
                      .arg(QDateTime::currentDateTime().toString("dd-MM-yy hh:mm:ss"))
                      ;
        label->setText(str);
        FrameNo->setText("-");
        appLog::write(LOG_LEVEL_CAM_MON, str );
        activateCamMonitor();
    }
}

void MainWindow::onMonitorError()
{
    const cam_logger* clogger = loggers.at(appState.camId);
    if (isCamMonitorActive())
    {
        QString str = QString("%1 lost connection at %2")
                      .arg(clogger->get_name())
                      .arg(QDateTime::currentDateTime().toString("dd-MM-yy hh:mm:ss"))
                      ;
        appLog::write(LOG_LEVEL_CAM_MON, str);
        label->setText(str);
        activateSelf();
    }

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

#if !defined DESKTOP_DEBUG_BUILD && !defined NOT_RASPBERRY
    wiringPiSetup();

    pinMode(PIN_SWITCH, INPUT);
    pullUpDnControl(PIN_SWITCH, PUD_UP);

    pinMode(PIN_LED1, OUTPUT);


    //init fan control gpio pin
    pinMode(PIN_FAN, OUTPUT);
    pullUpDnControl(PIN_FAN, PUD_DOWN);
    fanSwitch(true);
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
    QByteArray app = QCoreApplication::applicationFilePath().toLocal8Bit();
    QByteArray   app_upd = get_update_file_name().toLocal8Bit();

    if (QFile(app_upd).exists())
    {
        ret =
            (0 == unlink(app.constData()) &&
             0 == rename(app_upd.constData(), app.constData())
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

QString MainWindow::version()
{
    QString versionString =
        versionString.asprintf("%d.%02d", VHUB_VERSION_MAJOR, VHUB_VERSION_MINOR);
    return  versionString;
}
