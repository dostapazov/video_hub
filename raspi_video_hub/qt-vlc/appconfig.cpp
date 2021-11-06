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
    if (config)
        return config->getValue(key);
    return QVariant();
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

bool     appConfig::get_cam_logdisabled (QString camName)
{
    QVariant v = value(QString("/%1/LOGDISABLED").arg(camName));
    return   v.isValid() ? v.toBool() : false;
}

