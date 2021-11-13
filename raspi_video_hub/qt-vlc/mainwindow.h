#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma once

#include <QSerialPort>
#include <QTimer>
#include <QMap>
#include <QWindow>

#include "ui_mainwindow.h"
#include "filedeleterthread.h"
#include "cam_logger_vlc.h"
#include "vlcclasses.hpp"
#include "proto.h"
#include <functional>


#define PIN_LED1    3
#define PIN_SWITCH  0
#define PIN_FAN     4

#define FAN_ON      1
#define FAN_OFF     0

#define SWITCH_JABBER 30    //switch pin jabber timeout

#define UPDATE_EXIT_CODE 77

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = Q_NULLPTR);
    ~MainWindow();

Q_SIGNALS:
    void cam_switch(quint8 cam_id);

private slots:

    void onUARTread();
    void onParse();

    void on_blink();
    void onCamSwitch(quint8 camNum);
    void mon_player_events    (const libvlc_event_t event);
    void onPlayerResponseTimeout();
    void on_bTestUpdate_clicked();



private:
    void closeEvent(QCloseEvent* event) override;
    void showEvent  (QShowEvent*   event) override;
#ifdef DESKTOP_DEBUG_BUILD
    void keyReleaseEvent(QKeyEvent* event) override;
#endif


    void initBlinker();
    void createPlayer();
    void releasePlayer();

    void start_cam_monitor();
    void init_gpio  ();
    void init_libvlc();
    void load_config();
    void init_uart  ();
    void deinitUART ();
    void deinit_all ();
    void start_loggers();
    void start_file_deleter();
    void readCPUtemper();
    QString whoami();
    bool check_media_drive();
    void handle_uart_packet(PCK_Header_t& header, int offset);
    void setSystemDateTime(QDateTime dt);
    void initPlayer();

    void onPlayerStoped(vlc::vlc_player* player);
    void onPlayerPlaying(vlc::vlc_player* player);
    void onPlayerError(vlc::vlc_player* player);
    void onPlayerPoschanging(vlc::vlc_player* player);


    using  player_events_handler_t = std::function<void(vlc::vlc_player*)>;
    using  PlayerHandlers = QMap<libvlc_event_e, player_events_handler_t>;
    QString m_vlog_root  ;

    PlayerHandlers playerHandlers;
    vlc::vlc_player*      m_mon_player   = Q_NULLPTR;

    static constexpr int PLAYER_RESPONSE_TIMEOUT = 10000;
    QTimer playerResponseTimer;

    QList<cam_params_t> readCameraList();
    QVector<cam_logger_vlc*>   loggers;
    cam_logger_vlc* mon_logger = nullptr;

    QTimer blinker ;
    QTimer parser  ;

    int                led_state = 1;

    QByteArray         rxBuf;
    QSerialPort*       uart = Q_NULLPTR;

    FileDeleterThread* file_deleter = Q_NULLPTR;
    QWidget   m_playerWindow;

    int             cam_time_synchro = -1;
    PCK_STATE_t     appState;
    static const char* const vlcArgs[];
    static bool    do_rename_recorder  ();
    static QString get_update_file_name();
    static void    check_need_update   ();



};

#endif // MAINWINDOW_H
