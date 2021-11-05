#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QStandardItemModel>
#include <QSettings>
#include <QTcpSocket>
#include "memoryini.h"
#include "libssh2.h"
#include "libssh2_sftp.h"
#include "ui_mainwindow.h"

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT


    explicit MainWindow(QWidget *parent = nullptr);


public:
    virtual ~MainWindow();
      bool is_connected();
static MainWindow * get_instance    ();

private:
    QStringList sftpListDir(LIBSSH2_SFTP* sess, QString path, bool withFolders = false);
    QByteArray sftpFileRead(LIBSSH2_SFTP *sess, QString path);
    void fillInterface();
    bool write_update_file(const QString & src_file, const QString & dest_file);
    bool write_update_command();

protected:
    void changeEvent(QEvent *e);
    static  void disconnect_callback(LIBSSH2_SESSION *session, int reason, const char *message,
                                     int message_len, const char *language, int language_len,
                                     void **abstract);
    void dev_disconnect         ();
    bool dev_connect            (QString host, int port);
    void enable_connect_controls(bool enable);


private slots:
    void onChanged();
    void onCamDataChanged();
    void onCamSelectionChanged();
    void on_btConnect_clicked();

    void on_btDownload_clicked();

    void on_pushButtonSave_clicked();

    void on_btDel_clicked();

    void on_btAdd_clicked();

    void on_edHost_currentTextChanged(const QString &arg1);

    void on_btUpdate_clicked();

    void on_actionEN_triggered();

    void on_actionRU_triggered();

private:
    LIBSSH2_SESSION *session = nullptr;
    int rc = 0;
    LIBSSH2_SFTP *sftp_session = nullptr;
    LIBSSH2_SFTP_HANDLE *sftp_handle = nullptr;
    //LIBSSH2_CHANNEL *channel = nullptr;
    QTcpSocket socket;

    memoryIni config;
    QByteArray rawCfg;
    QStandardItemModel camList;
    static MainWindow * instance;
    static quint16 port     ;
    static QString sftpfile ;
    static QString vlogPath ;
    static QString title    ;

    QTranslator qtLanguageTranslator;
};

#endif // MAINWINDOW_H
