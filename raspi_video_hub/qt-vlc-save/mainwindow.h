#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma once

#include <QSerialPort>
#include <QTimer>
#include <QMap>
#include <QWindow>

#include "ui_mainwindow.h"
#include "filedeleterthread.h"
#include "cam_logger.h"
#include "recvparser.hpp"
#include "proto.h"
#include <functional>

#define PIN_LED1    3
#define PIN_SWITCH  0
#define PIN_FAN     4

#define FAN_ON      1
#define FAN_OFF     0

#define UPDATE_EXIT_CODE 77

constexpr int VHUB_VERSION_MAJOR = 2;
constexpr int VHUB_VERSION_MINOR = 1;
constexpr int SEND_STATE_PERIOD  = 5000;

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = Q_NULLPTR);
    ~MainWindow();

Q_SIGNALS:
    void cam_switch(quint8 cam_id);

private slots:

    void on_blink();
    void onCamSwitch(quint8 camId);
    void on_bTestUpdate_clicked();
    void onFramesChanged(int displayFrames, int lostFrames);
    void onMonitorError();
    void setSystemDateTime(QDateTime dt);
    void startLoggers();

    void sendCamState();
    void errorPacket(QByteArray packet, bool crc);

private:
    void closeEvent(QCloseEvent* event) override;
#ifdef DESKTOP_DEBUG_BUILD
    void keyReleaseEvent(QKeyEvent* event) override;
#endif
    quint8 devId;
    QString m_vlog_root  ;
    QWidget* m_CamWidget = nullptr;

    QList<cam_params_t> readCameraList();
    QVector<cam_logger*>   loggers;
    cam_logger* cam_monitor = nullptr;
    quint64 m_FramesDisplayed, m_FramesLost;
    void onStartMon();

    QTimer blinker ;
    QTimer starLoggersTimer ;
    QTimer stateTimer;

    RecvParser  recvParser;
    PCK_STATE_t appState ;

    bool led_state = true;

    QSerialPort*       uart = Q_NULLPTR;
    FileDeleterThread* file_deleter = Q_NULLPTR;

    void initBlinker();
    void startCamMonitor();
    void init_gpio  ();
    void init_libvlc();
    void load_config();
    void init_uart  ();

    void deinitUART ();
    void deinitLoggers();
    void deinit_all ();

    void start_file_deleter();
    void readCPUtemper();
    bool fanSwitch(bool on);
    QString whoami();
    bool check_media_drive();

    static const char* const vlcArgs[];
    static bool    do_rename_recorder  ();
    static QString get_update_file_name();
    static void    check_need_update   ();

    void deinitMonitor();
    void deinitFileDeleter();
    void initStartLoggers();
    void initRecvParser(QIODevice* io);
    void initCamMonitor();

    bool isCamMonitorActive();
    void activateCamMonitor();
    void activateSelf();

    static QString version();
};

#if defined DESKTOP_DEBUG_BUILD || defined (NOT_RASPBERRY)
    #define digitalRead(x)
    #define digitalWrite(x,y)
#else
    #include <wiringPi.h>

#endif


#endif // MAINWINDOW_H
