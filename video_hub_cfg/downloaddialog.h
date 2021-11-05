#ifndef DOWNLOADDIALOG_H
#define DOWNLOADDIALOG_H

#include "libssh2.h"
#include "libssh2_sftp.h"
#include "ui_downloaddialog.h"
#include <qprogressbar.h>

class DownloadDialog : public QDialog, public Ui::DownloadDialog
{
    Q_OBJECT

public:
    explicit DownloadDialog(LIBSSH2_SESSION** sess, QWidget *parent = nullptr);
    ~DownloadDialog();
    void setPath(QString path);

private:
    void drawRootPath();

protected:
    void changeEvent  (QEvent *e);
    int  remove_files (const QList<QListWidgetItem*> & files);
    bool remove_file  (const QString file_name);
    int  copy_files   (const QList<QListWidgetItem*> & files);
    bool copy_file    (const QString &src_name, const QString &dst_name, QProgressBar *pb = Q_NULLPTR);
    void scan_folder  (const QString & folder );
    bool sftp_init    ();
    void sftp_close   ();
    void switch_frames();


private slots:
    void on_lwFiles_itemDoubleClicked(QListWidgetItem *item);
    void on_bRemoveFiles_clicked();

    void on_lwFiles_itemSelectionChanged();

    void on_bCopy_clicked();

private:
    QString lastPath;
    LIBSSH2_SESSION  **main_sess    = Q_NULLPTR;
    LIBSSH2_SFTP      *sftp_session = Q_NULLPTR;
    QString rootPath;
};

#endif // DOWNLOADDIALOG_H
