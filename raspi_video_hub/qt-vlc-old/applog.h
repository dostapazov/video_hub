#ifndef APPLOG_H
#define APPLOG_H

#include <QtCore>
#include <QObject>
#include <QFile>

class appLog : public QObject
{
    Q_OBJECT
public:
    explicit appLog(QString logFileName,int level, QObject *parent = 0);
    void logWrite(int level, QString message);
    void endLog();

    static void init(QString fName,int level);
    static void write(int level,QString mess);
    static void deinit();

private:
    QFile logFile;
    int   level;
};

#endif // APPLOG_H
