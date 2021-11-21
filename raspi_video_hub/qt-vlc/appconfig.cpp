#include "appconfig.h"

static appConfig* config = Q_NULLPTR;

//
//
//
appConfig::appConfig(QObject* parent)
    : QObject(parent)
{

}

void appConfig::open(const QString& organization, const QString& application)
{
    if (cfg)
        return;

    cfg = new QSettings(organization, application);
}

QVariant appConfig::getValue(QString key)
{
    if (cfg)
    {
        cfg->sync();
        return cfg->value(key);
    }
    return QVariant();
}

void appConfig::putValue(QString key, QVariant value)
{
    if (cfg)
        cfg->setValue(key, value);
}

//
//static members
//

void appConfig::init(const QString organization, const QString application, QObject* parent)
{
    if (config)
        return;
    config = new appConfig(parent);
    config->open(organization, application);
}

QVariant appConfig::value(QString key)
{
    if (!config)
        return QVariant();
    return  config->getValue(key);
}

void appConfig::setValue(QString key, QVariant value)
{
    if (config)
        config->putValue(key, value);
}

int appConfig::get_cam_id(QString camName)
{
    return  value(QString("/%1/ID").arg(camName)).toInt();
}

QString     appConfig::get_cam_name(QString camName)
{
    return  value(QString("/%1/Name").arg(camName)).toString();
}

QString     appConfig::get_cam_mrl (QString camName)
{
    return  value(QString("/%1/MRL").arg(camName)).toString();
}

int         appConfig::get_time_duration()
{
    constexpr int ONE_HOUR = 60;
    int duration = appConfig::value("COMMON/video_length").toInt();
    return duration ? duration : ONE_HOUR;

}


bool     appConfig::get_cam_logdisabled (QString camName)
{
    QVariant v = value(QString("/%1/LOGDISABLED").arg(camName));
    return   v.isValid() ? v.toBool() : false;
}


QString     appConfig::get_mount_point()
{
    return value("VLOG/MountPoint").toString();
}

QString     appConfig::get_vlog_folder()
{
    return value("VLOG/Folder").toString();
}

void        appConfig::set_vlog_folder(const QString& folder)
{
    setValue("VLOG/Folder", folder);
}

void        appConfig::set_log_enabled(bool enabled)
{
    setValue("VLOG/Enabled", enabled);
}

QString     appConfig::get_log_name()
{
    constexpr const char* key = "TLOG/FILE";
    QString logName = value(key).toString();
    if (logName.isEmpty())
    {
        logName =   "vhub.log";
        setValue(key, logName);
    }
    return logName;
}

int         appConfig::get_log_level()
{
    constexpr const char* key = "TLOG/LEVEL";
    QString level = value(key).toString();
    if (level.isEmpty())
    {
        level = "100";
        setValue(key, level.toInt());
    }
    return level.toInt();
}



