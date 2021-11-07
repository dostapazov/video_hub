#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QSettings>
#include <QObject>

class appConfig : public QObject
{
    Q_OBJECT
public:
    explicit appConfig(QObject* parent = Q_NULLPTR);
    void open(const QString& organization, const QString& application = QString());
    QVariant getValue(QString key);
    void putValue(QString key, QVariant value);

//static members

    static void        init(const QString organization, const QString application = QString(), QObject* parent = Q_NULLPTR);
    static QVariant    value(QString key);
    static void        setValue(QString key, QVariant value);
    static int         get_disc_free_space_percent() { return value("COMMON/diskfreespace").toInt() ;}
    static int         get_mon_camera     () { return value("DEV/CAMERA").toInt();}
    static QString     get_uart_device    () { return value("USART/DEVICE").toString();}
    static quint32     get_uart_speed     () { return value("USART/BAUD").toUInt();}
    static quint8      get_devid          () { return static_cast<quint8>(value("DEV/ID").toUInt());}
    static QStringList get_cam_list       () { return value("/DEV/LIST").toStringList();}
    static int         get_time_duration();
    static int         get_cam_id  (QString camName);
    static QString     get_cam_name(QString camName);
    static QString     get_cam_mrl (QString camName);
    static bool        get_cam_logdisabled (QString camName);
    static QString     get_mount_point();
    static QString     get_log_folder();
    static QString     get_log_name();
    static int         get_log_level();




private:
    QSettings* cfg = Q_NULLPTR;
};

#endif // APPCONFIG_H
