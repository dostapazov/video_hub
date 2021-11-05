#ifndef MEMORYINI_H
#define MEMORYINI_H

#include <QtCore>
#include <QSettings>

class memoryIni : public QObject
{
    Q_OBJECT
public:
    explicit memoryIni(QByteArray* ds = nullptr, QObject *parent = nullptr);
    void setDataSource(QByteArray* ds);

    QVariant value(QString key);
    void setValue(QString key, QVariant value);
    void delValue(QString key);
    void delGroup(QString group);

    QStringList allKeys();
    QVariantList allValues();
    QStringList groups();

    void push();

private:
    void parse();

signals:

public slots:

private:
    QByteArray* source;
    QSettings::SettingsMap content;
    QList<QByteArray> cfgSections;
    QStringList modified;
};

#endif // MEMORYINI_H
