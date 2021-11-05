#include <QPushButton>
#include "camadddialog.h"

camAddDialog::camAddDialog(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);
    pushButtonAccept->setEnabled(false);
}

void camAddDialog::setExistsData(QStringList names, QStringList sysNames, QList<quint8> ids)
{
    namesList = names;
    sysNamesList = sysNames;
    idList = ids;
    onChanged();
}

void camAddDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
        case QEvent::LanguageChange:
            retranslateUi(this);
            break;
        default:
            break;
    }
}

void camAddDialog::onChanged()
{
    bool ok = !(edName->text().isEmpty() || edSysName->text().isEmpty() || edMRL->text().isEmpty());

    if (namesList.contains(edName->text().trimmed()))
    {
        edName->setStyleSheet("color: rgb(255, 0, 0);");
        ok = false;
    }
    else
        edName->setStyleSheet("");

    if (sysNamesList.contains(edSysName->text()))
    {
        edSysName->setStyleSheet("color: rgb(255, 0, 0);");
        ok = false;
    }
    else
        edSysName->setStyleSheet("");

    if (idList.contains(sbID->value()))
    {
        sbID->setStyleSheet("color: rgb(255, 0, 0);");
        ok = false;
    }
    else
        sbID->setStyleSheet("");

    pushButtonAccept->setEnabled(true);
}
