#include <QMap>
#include "memoryini.h"


memoryIni::memoryIni(QByteArray *ds, QObject *parent) : QObject(parent)
{
    if (ds)
        setDataSource(ds);
}

void memoryIni::setDataSource(QByteArray* ds)
{
    source = ds;
    if (!source) return;
    parse();
}

QVariant memoryIni::value(QString key)
{
    return content.value(key);
}

void memoryIni::setValue(QString key, QVariant value)
{
    content[key] = value;
}

void memoryIni::delValue(QString key)
{
    content.remove(key);
}

void memoryIni::delGroup(QString group)
{
    QStringList todel;
    foreach(QString grp, content.keys())
    {
        if (grp.left(group.count()) == group)
            todel.append(grp);
    }

    qDebug() << Q_FUNC_INFO << group << todel;

    foreach(QString key, todel)
        content.remove(key);
}

QStringList memoryIni::allKeys()
{
    return content.keys();
}

QVariantList memoryIni::allValues()
{
    return content.values();
}

QStringList memoryIni::groups()
{
    QStringList res;
    foreach(QString key, content.keys())
    {
        int idx = key.indexOf('/');
        if (idx)
        {
            QString g = key.left(idx);
            if (!res.contains(g))
                res.append(g);
        }
    }
    return res;
}

void memoryIni::push()
{
    if (!source) return;
    source->clear();
    source->resize(0);

    QStringList grp = groups();
    QStringList ks = allKeys();

    foreach(QString group, grp)
    {
        source->append(QString("[%1]\n").arg(group).toLocal8Bit());
        foreach(QString key, ks)
        {
            if (key.left(group.count()) == group)
                source->append(QString("%1=%2\n").arg(key.mid(group.count()+1, key.count()-group.count()-1)).arg(content.value(key).toString()).toLocal8Bit());
        }

        if (!source->isEmpty())
            source->append("\n\n");
    }
}

void memoryIni::parse()
{
    content.clear();
    if (!source) return;
    if (source->isEmpty()) return;

    QByteArrayList lines = source->split('\n');
    QByteArray prefix;
    foreach (QByteArray l, lines)
    {
        QByteArray ln = l.trimmed();
        if (!ln.isEmpty())
        {
            if ((ln[0] == '[') && (ln[ln.count()-1] == ']')) // block
            {
                prefix = ln.mid(1, ln.count()-2);
                cfgSections.append(prefix);
            }
            else //value
            {
                int idx = ln.indexOf('=');
                if (idx)
                {
                    QString value = QString(ln.right(ln.count()-idx-1)).trimmed();
                    if ((value[0]=='"') && (value[value.count()-1] == '"'))
                        value = value.mid(1, value.count()-2);
                    content.insert(prefix+"/"+ln.left(idx), value);
                }
            }
        }
    }
}
