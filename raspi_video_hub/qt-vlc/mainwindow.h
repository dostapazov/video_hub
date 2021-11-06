#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma once

#include <QSerialPort>
#include <QTimer>
#include "ui_mainwindow.h"
#include "camtimesync.h"
#include "filedeleterthread.h"
#include "cam_logger_vlc.h"
#include "vlcclasses.hpp"
#include "proto.h"

#define PIN_LED1    3
#define PIN_SWITCH  0
#define PIN_FAN     4

#define FAN_ON      1
#define FAN_OFF     0

#define SWITCH_JABBER 30    //switch pin jabber timeout

#define UPDATE_EXIT_CODE 77

#define CAMERA_WDT_INTERVAL     20000
#define VLOG_WDT_INTERVAL       20*1000

//typedef struct CAM_CFG
//{
//    quint8  id;
//    QString mrl;
//    QString name;
//    QString opts;
//    CAM_CFG():id(0){};
//}CAM_CFG_t;


class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = Q_NULLPTR);
    ~MainWindow();


    void initBlinker();

    void createPlayer();

protected:
    void closeEvent(QCloseEvent* event);

private:

    void start_cam_monitor();
    void start_time_sync();
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
    void showEvent  (QShowEvent*   event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void start_cam_switch(bool enable);
    void handle_uart_packet(PCK_Header_t& header, int offset);
    void deinit_player();
Q_SIGNALS:
    void cam_switch(quint8 cam_id);

private:
    void setSystemDateTime(QDateTime dt);


private slots:

    void onUARTread();
    void onParse();

    void on_blink();
    void on_moncam_timeout();
    void on_monloger_timeout();
    void on_cam_switch(quint8 camNum);

    void cam_time_synchronized(bool ok);
    void cam_time_difference  (const QDateTime& dt, const qint64& diff);
    void mon_player_events    (const libvlc_event_t event);


    void on_bTestUpdate_clicked();

private:
    QList<cam_params_t> readCameraList();

    vlc::vlc_player*      m_mon_player   = Q_NULLPTR;
    QString m_vlog_root  = "d:/rasp_media/VLOG6";
    int     m_vlog_tmlen = 3600;
    QVector<cam_logger_vlc*>   loggers;

    quint8 camIndex = 0;
    QTimer blinker ;
    QTimer parser  ;
    QTimer moncam_timer;
    QTimer monlog_timer;
    int                led_state = 1;
    enum               vlogger_state_e {vl_disable, vl_enable, vl_working};
    vlogger_state_e    m_vlogger_state = vl_disable;

    QByteArray         rxBuf;
    QSerialPort*       uart = Q_NULLPTR;

    FileDeleterThread* file_deleter = Q_NULLPTR;

    CamTimeSync     cam_time_sync;
    int             cam_time_synchro = -1;
    PCK_STATE_t     appState;
    static const char* const vlcArgs[];
    static bool    do_rename_recorder  ();
    static QString get_update_file_name();
    static void    check_need_update   ();


    bool is_cam_online = false;

};

#endif // MAINWINDOW_H
