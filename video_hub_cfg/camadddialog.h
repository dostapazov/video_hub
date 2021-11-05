#ifndef CAMADDDIALOG_H
#define CAMADDDIALOG_H

#include "ui_camadddialog.h"

class camAddDialog : public QDialog, public Ui::camAddDialog
{
    Q_OBJECT

public:
    explicit camAddDialog(QWidget *parent = nullptr);
    void setExistsData(QStringList names, QStringList sysNames, QList<quint8> ids);

protected:
    void changeEvent(QEvent *e);

private slots:
    void onChanged();

private:
    QStringList namesList;
    QStringList sysNamesList;
    QList<quint8> idList;
};

#endif // CAMADDDIALOG_H
