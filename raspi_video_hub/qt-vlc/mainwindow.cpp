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
    appState = PCK_STATE_t {0xFF, 77, 777};
    connect(this, &MainWindow::cam_switch, this, &MainWindow::onCamSwitch, Qt::ConnectionType::QueuedConnection);
    init_gpio  ();
    initBlinker();
    init_libvlc      ();
    init_uart        ();
    load_config      ();
    start_loggers();

#if defined (DESKTOP_DEBUG_BUILD)
    show();
#else
    showFullScreen();
#endif

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
    cams.append({1, tr("IPCam100"), tr("rtsp://192.168.0.100:554/media/video1"), false});
    cams.append({1, tr("IPCam101"), tr("rtsp://192.168.0.101:554/media/video1"), false});
#else

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

#ifdef DESKTOP_DEBUG_BUILD
    appConfig::setValue("VLOG/Folder", "streaming");
#ifdef Q_OS_LINUX
    appConfig::setValue("VLOG/MountPoint", "/home/dostapazov/raspi_log/");
#else
    appConfig::setValue("VLOG/MountPoint", "d:/raspi_log/");
#endif

#endif
}

void MainWindow::deinit_all()
{
    qDebug() << Q_FUNC_INFO << " begin";

    blinker.stop ();
    disconnect(&blinker );

    foreach (cam_logger_vlc* cl, this->loggers)
    {
        cl->stop();
        cl->deleteLater();
    }
    loggers.clear();

    cam_monitor->stop();
    cam_monitor->deleteLater();
    cam_monitor = nullptr;


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
    addFolder(strPath, appConfig::get_log_folder());

    appLog::write(0,  tr("check existing streamig dir %1").arg(strPath));
    if ( QDir(strPath).exists() )
    {

        appLog::write(0, tr("directory exists. Begin logging video"));
        m_vlog_root = strPath;
    }
    else
    {
        appLog::write(0, tr("directory NOT exist: write videolog disabled "));
        m_vlog_root.clear();
    }
    return  !m_vlog_root.isEmpty();
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
        const cam_logger_vlc* clogger = loggers.at(camId);
        QString str = tr("Monitor switch to camera (%1) %2").arg(int(camId)).arg(clogger->get_mrl());
        qDebug() << str;
        appLog::write(6, str);

        m_camWindow.setWindowTitle(clogger->get_name());
        cam_monitor->startMonitoring(&m_camWindow, clogger->get_mrl());
        str = QString("Wait data from %1").arg(clogger->get_name());
        label->setText(str);
    }
    return;
}



void MainWindow::start_cam_monitor()
{
    appLog::write(2, "start_cam_monitor next must be start_cam_switch");
    cam_monitor = new cam_logger_vlc({-1, "Monitor logger", ""});
    connect(cam_monitor, &cam_logger_vlc::onStartMon, this, &MainWindow::onStartMon);
    connect(cam_monitor, &cam_logger_vlc::onStopMon, this, &MainWindow::onStopMon);
    connect(cam_monitor, &cam_logger_vlc::onError, this, &MainWindow::onMonitorError);
    connect(cam_monitor, &cam_logger_vlc::framesChanged, this, &MainWindow::onFramesChanged);

    int cam_id = std::max(appConfig::get_mon_camera(), 0);
    emit cam_switch(static_cast<quint8>(cam_id));
}

void MainWindow::onFramesChanged(int frames)
{
    this->FrameNo->setText(QString::number(frames));
}


void MainWindow::onStartMon()
{
    const cam_logger_vlc* clogger = loggers.at(appState.camId);
    QString str = QString("Play from  %1").arg(clogger->get_name());
    label->setText(str);
    FrameNo->setText("-");
#if defined DESKTOP_DEBUG_BUILD
    m_camWindow.show();
#else
    m_camWindow.showFullScreen();
#endif
    m_camWindow.activateWindow();

}

void MainWindow::onStopMon()
{
    const cam_logger_vlc* clogger = loggers.at(appState.camId);
    QString str = QString("%1 lost connection").arg(clogger->get_name());
    appLog::write(6, str);
    label->setText(str);
    m_camWindow.hide();
}

void MainWindow::onMonitorError()
{
    const cam_logger_vlc* clogger = loggers.at(appState.camId);

    bool monWidgetVisible = m_camWindow.isVisible();

    if (monWidgetVisible)
    {
        QString str = QString("Camera %1 not respond").arg(clogger->get_name());
        appLog::write(0, str);
        m_camWindow.hide();
    }
    cam_monitor->startMonitoring(&m_camWindow, clogger->get_mrl());
}

void MainWindow::start_loggers()
{
    if (m_vlog_root.isEmpty() && check_media_drive())
    {
        start_file_deleter();
        int timeDuration = appConfig::get_time_duration();

        foreach (cam_logger_vlc* cl, loggers)
        {
            cl->startStreaming(m_vlog_root, timeDuration);
        }
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

