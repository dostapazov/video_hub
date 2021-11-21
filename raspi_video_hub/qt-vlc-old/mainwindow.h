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


//typedef struct CAM_CFG
//{
//    quint8  id;
//    QString mrl;
//    QString name;
//    QString opts;
//    CAM_CFG():id(0){};
//}CAM_CFG_t;

typedef cam_logger_vlc logger_t;


class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = Q_NULLPTR);
    ~MainWindow();


protected:
    void closeEvent(QCloseEvent *event);

private:

    void start_cam_monitor();
    void start_time_sync();
    void init_gpio  ();
    void init_libvlc();
    void init_config();
    void init_uart  ();
    void deinitUART ();
    void deinit_all ();
    void start_loggers();
    void start_file_deleter();
    void readCPUtemper();
 QString whoami();
    bool check_media_drive();
    void showEvent  (QShowEvent  * event);
    void start_cam_switch(bool enable);
    void handle_uart_packet(PCK_Header_t & header, int offset);
Q_SIGNALS:
       void cam_switch(quint8 cam_id);

private slots:

    void onUARTread();
    void onParse();

    void on_blink();
    void on_moncam_timeout();
    void on_cam_switch(quint8 camNum);

    void cam_time_synchronized(bool ok);
    void cam_time_difference  (const QDateTime& dt, const qint64 & diff);
    void mon_player_events    (const libvlc_event_t event);


    void on_bTestUpdate_clicked();

private:

    vlc::vlc_player     * m_mon_player   = Q_NULLPTR;
    QString m_vlog_root  = "d:/rasp_media/VLOG6";
    int     m_vlog_tmlen = 3600;
    QVector<logger_t *>   loggers;

    quint8 camIndex = 0;
    QTimer blinker ;
    QTimer parser  ;
    QTimer moncam_timer;
    int                led_state = 1;
    enum               vlogger_state_e {vl_disable,vl_enable,vl_working};
    vlogger_state_e    m_vlogger_state = vl_disable;

    QByteArray         rxBuf;
    QSerialPort      * uart = Q_NULLPTR;

    FileDeleterThread * file_deleter = Q_NULLPTR;

    CamTimeSync     cam_time_sync;
    int             cam_time_synchro = -1;
    PCK_STATE_t appState;
    static const char* const vlcArgs[];
    static bool    do_rename_recorder  ();
    static QString get_update_file_name();
    static void    check_need_update   ();

};

#endif // MAINWINDOW_H
