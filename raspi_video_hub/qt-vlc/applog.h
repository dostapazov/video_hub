#ifndef APPLOG_H
#define APPLOG_H

#include <QtCore>
#include <QObject>
#include <QFile>

class appLog : public QObject
{
    Q_OBJECT
public:
    explicit appLog(QString logFileName, int level, QObject* parent = 0);
    void logWrite(int level, QString message);
    void endLog();

    static void init(QString fName, int level);
    static void write(int level, QString mess);
    static void deinit();

private:
    QFile logFile;
    int   level;
};

constexpr int LOG_LEVEL_DEBUG = 1;
constexpr int LOG_LEVEL_UART = 2;
constexpr int LOG_LEVEL_VLC = 4;
constexpr int LOG_LEVEL_CAM_MON = 8;
constexpr int LOG_LEVEL_PARSER = 16;
constexpr int LOG_LEVEL_FAN_CTRL = 32;
constexpr int LOG_LEVEL_ALWAYS = 0;

#endif // APPLOG_H
