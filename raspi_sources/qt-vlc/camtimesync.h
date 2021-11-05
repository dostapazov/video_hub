/*******************************************
 * Class CamTimeSync
 * designed for get time value from corresponding cameras
 * Interface
 * Ostapenko D. V. 2018-10-30 NIKTES
 * ******************************************/



#ifndef CAMTIMESYNC_H
#define CAMTIMESYNC_H

#include <qnetworkaccessmanager.h>
#include <qnetworkreply.h>
#include <qtimer.h>


class CamTimeSync : public QObject
{
    Q_OBJECT
public:
    explicit CamTimeSync (QObject *parent = nullptr);
    ~CamTimeSync(){close_reply();}
    bool start_sync(const QString &url_string, int interval = 5000);
    void stop_sync ();
    void schedule_next_request(int interval = 5000);

    QString   host     ()             {return m_host;}
    QDateTime date_time()             {return m_date_time;}
    int       sec_delta()             {return m_sec_delta;}
    void      set_sec_delta(int delta){m_sec_delta = delta;}

signals:
    void synchronized   (bool ok);
    void time_difference(const QDateTime & sync_tyme,const qint64 &delta);
private slots:

    void replay_error       (QNetworkReply::NetworkError);
    void replay_finished    ();
    void replay_meta_changed();
    void timer_fired        ();


public slots:

private :
    bool request_cam_time();

    QNetworkAccessManager access_mgr;
    QTimer     m_timer;
    QString    m_host;
    int        m_sec_delta = 5;
    QDateTime  m_date_time ;
    void close_reply();
    QNetworkReply  *m_reply;

};

#endif // CAMTIMESYNC_H
