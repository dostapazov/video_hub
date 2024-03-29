#include <QDebug>
#include <QFileInfo>
#include "applog.h"


static appLog* applog = Q_NULLPTR;

appLog::appLog(QString logFileName, int _level, QObject *parent) : QObject(parent)
{
    QFileInfo info(logFileName);
    logFile.setFileName(info.absoluteFilePath());
    this->level = _level;
    logWrite(0,"--- Starting ---");
}

void appLog::logWrite(int log_level,QString message)
{
    if(level<log_level) return;

    qDebug() << QString("level %1 : ").arg(log_level) << message;
    logFile.open(QFile::Text | (logFile.exists()?QFile::Append:QFile::WriteOnly));
    if(logFile.size()>256000)
    {
        logFile.resize(0);
        logFile.seek(0);
     }
    QString mess = QString("%1 |L-%2| %3 \r\n").arg(QDateTime::currentDateTime().toString()).arg(log_level,3).arg(message);
    logFile.write(mess.toUtf8().constData());

    logFile.close();
}

void appLog::endLog()
{
    if (logFile.isOpen())
        logFile.close();
}

void appLog::init(QString fName,int log_level)
{
    if (!applog)
        applog = new appLog(fName,log_level);
}

void appLog::write(int level,QString mess)
{
    if (applog)
        applog->logWrite(level,mess);
}

void appLog::deinit()
{
    if (applog)
    {
        applog->endLog();
        applog->deleteLater();
    }
}
