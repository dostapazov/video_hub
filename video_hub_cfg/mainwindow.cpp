#include <QDebug>
#include <QMessageBox>
#include <QSettings>
#include <QPushButton>
#include "camadddialog.h"
#include "downloaddialog.h"
#include "mainwindow.h"
#include <QFileDialog>
#include <qstatusbar.h>

#ifdef Q_OS_WIN
#include "fileinfo.h"
#endif

 quint16 MainWindow::port     = 22;
 QString MainWindow::sftpfile = "/home/pi/.config/NIKTES/Simple Video HUB.conf";
 QString MainWindow::vlogPath = "";
 QString MainWindow::title    = tr("Simple video hub remote configurator");

//
//
//
enum
{
    COL_SYS_NAME = 0,
    COL_ID,
    COL_MRL,
    COL_NAME,

    COL_END
};

//
//
//

MainWindow * MainWindow::instance = Q_NULLPTR;

MainWindow * MainWindow::get_instance()
{
  if(!instance)
      instance = new MainWindow();
  return instance;
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setupUi(this);
    camList.setColumnCount(COL_END);
    lwCamNames->setModel(&camList);
    lwCamNames->setModelColumn(0);

    connect(lwCamNames->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), SLOT(onCamSelectionChanged()));

    QString winTitle = title;
#ifdef Q_OS_WIN
    winTitle += " v "+verIntToStr(fileVersion(qApp->applicationFilePath()).longVer);
#endif

    setWindowTitle(winTitle);

    /*
    channel = libssh2_channel_open_session(session);
    qDebug() << "EXEC" << libssh2_channel_exec(channel, "reboot");
    libssh2_channel_free(channel);
    */

    QMenu* menu = new QMenu(tr("&Language"));
    menu->addAction(actionEN);
    menu->addAction(actionRU);
    Ui::MainWindow::menuBar->addMenu(menu);

    actionEN->trigger();

    enable_connect_controls(false);
}

void MainWindow::disconnect_callback(LIBSSH2_SESSION *session, int reason, const char *message,
                      int message_len, const char *language, int language_len,
                      void **abstract)
{

    Q_UNUSED(session)
    Q_UNUSED(reason)
    Q_UNUSED(message)
    Q_UNUSED(message_len)
    Q_UNUSED(language)
    Q_UNUSED(language_len)
    Q_UNUSED(abstract)
    instance->session = Q_NULLPTR;
    qDebug()<<reason<<' '<<message;
}


MainWindow::~MainWindow()
{

    dev_disconnect();
    if(instance == this) instance = Q_NULLPTR;
}

QStringList MainWindow::sftpListDir(LIBSSH2_SFTP *sess, QString path, bool withFolders)
{
    QStringList result;
    char mem[512];
    char longentry[512];
    LIBSSH2_SFTP_ATTRIBUTES attrs;
    int res = 0;

    LIBSSH2_SFTP_HANDLE* handle = libssh2_sftp_opendir(sess, path.toLocal8Bit());
    if(handle)
    {
     do
     {
        res = libssh2_sftp_readdir_ex(handle, mem, sizeof(mem),longentry, sizeof(longentry), &attrs);
        if (res)
        {
            //qDebug() << attrs.permissions << ((attrs.permissions & 0x4000) == 0) << longentry << res;
            if ((((attrs.permissions & 0x4000) == 0) || withFolders) && !((mem[0] == '.')/* && (res < 2)*/) )
                result.append(QString(mem));
        }

     } while (res);
     libssh2_sftp_close(handle);
    }
    return result;
}

QByteArray MainWindow::sftpFileRead(LIBSSH2_SFTP *sess, QString path)
{
    QByteArray result;
    char mem[4096];
    LIBSSH2_SFTP_HANDLE* handle = libssh2_sftp_open(sess, path.toLocal8Bit(), LIBSSH2_FXF_READ, 0);
    int res = 0;
    do
    {
        res = libssh2_sftp_read(handle, mem, sizeof(mem));
        if (res)
            result.append(QByteArray(mem, res));
    }while(res);
    return result;
}

void MainWindow::fillInterface()
{
    lwCamNames->model()->removeRows(0, lwCamNames->model()->rowCount());
    sbDevID->setValue(config.value("DEV/ID").toInt());
    cbDev->setCurrentText(config.value("USART/DEVICE").toString().trimmed());
    //edUARTDevice->setText(config.value("USART/DEVICE").toString());
    sbUARTBaud->setValue(config.value("USART/BAUD").toInt());
    sbFanStart->setValue(config.value("FAN/StartTemper").toInt());
    sbFanStop->setValue(config.value("FAN/StopTemper").toInt());

    QStringList clist = config.value("DEV/LIST").toString().split(",");
    foreach(QString sys_name, clist)
    {
        QList<QStandardItem*> row;
        QString name = config.value(QString("%1/Name").arg(sys_name.trimmed())).toString();
        QString mrl = config.value(QString("%1/MRL").arg(sys_name.trimmed())).toString();
        quint8 id = config.value(QString("%1/ID").arg(sys_name.trimmed())).toUInt();

        //qDebug() << Q_FUNC_INFO << sys_name << id << name << mrl;

        if (!name.isEmpty() && !mrl.isEmpty() && (id > 0))
        {
            QList<QStandardItem*> row;
            row << new QStandardItem(sys_name.trimmed()) << new QStandardItem(QString("%1").arg(id)) << new QStandardItem(mrl) << new QStandardItem(name);
            camList.appendRow(row);
        }
    }

/*
[CAM2]
DISPLAY=true
ID=2
MRL=rtsp://192.168.0.103:554/media/video1
Name=IP-Camera2
STREAMING=RTP2
*/
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
        case QEvent::LanguageChange:
    {
            retranslateUi(this);

            QString winTitle = tr("Simple video hub remote configurator");
#ifdef Q_OS_WIN
            winTitle += " v " + verIntToStr(fileVersion(qApp->applicationFilePath()).longVer);
#endif
            setWindowTitle(winTitle);

            Ui::MainWindow::menuBar->clear();
            QMenu* menu = new QMenu(tr("&Language"));
            menu->addAction(actionEN);
            menu->addAction(actionRU);
            Ui::MainWindow::menuBar->addMenu(menu);
    }
            break;
        default:
            break;
    }
}

void MainWindow::onChanged()
{
    config.push();
    pushButtonSave->setEnabled(true);
    pushButtonReject->setEnabled(true);
}

void MainWindow::onCamDataChanged()
{
    int row = lwCamNames->currentIndex().row();
    if (row < 0) return;

    camList.item(row, COL_ID)->setText(QString("%1").arg(sbCamID->value()));
    camList.item(row, COL_MRL)->setText(edMRL->text());
    camList.item(row, COL_NAME)->setText(edName->text());

    QStringList cams;
    for (int i = 0; i < camList.rowCount(); i++)
    {
        config.setValue(QString("%1/ID").arg(camList.item(row, COL_SYS_NAME)->text()), camList.item(row, COL_ID)->text());
        config.setValue(QString("%1/MRL").arg(camList.item(row, COL_SYS_NAME)->text()), camList.item(row,COL_MRL)->text());
        config.setValue(QString("%1/Name").arg(camList.item(row, COL_SYS_NAME)->text()), camList.item(row, COL_NAME)->text());
        cams.append(camList.item(i, COL_SYS_NAME)->text());
    }
    config.setValue("DEV/LIST",cams.join(","));

    onChanged();
}

void MainWindow::onCamSelectionChanged()
{
    btDownload->setEnabled(false);
    int row = lwCamNames->currentIndex().row();
    if (row < 0) return;

    sbCamID->blockSignals(true);
    edMRL->blockSignals(true);

    sbCamID->setValue(camList.item(row, COL_ID)->data(Qt::DisplayRole).toInt());
    edMRL->setText(camList.item(row, COL_MRL)->text());
    edName->setText(camList.item(row, COL_NAME)->text());

    sbCamID->blockSignals(false);
    edMRL->blockSignals(false);

    btDownload->setDisabled(vlogPath.isEmpty());
}

void MainWindow::enable_connect_controls(bool connected)
{
    edUser->setEnabled    (!connected);
    edPassword->setEnabled(!connected);
    btConnect->setEnabled (!connected);
    tbwConfig->setEnabled (connected);
    pushButtonSave->setEnabled(connected);
    pushButtonReject->setEnabled(connected);
    btUpdate->setEnabled  (connected);
}

void MainWindow::dev_disconnect()
{
  enable_connect_controls(false);

  if(sftp_handle)
  {
    libssh2_sftp_close_handle(sftp_handle);
    sftp_handle = Q_NULLPTR;
  }

  if (sftp_session)
  {
      libssh2_sftp_shutdown    (sftp_session);
      sftp_session = Q_NULLPTR ;
  }
  if (session)
  {
      libssh2_session_disconnect(session, "Normal Shutdown, Thank you for playing");
      libssh2_session_callback_set(session,LIBSSH2_CALLBACK_DISCONNECT,Q_NULLPTR);
      libssh2_session_free(session);
      session = Q_NULLPTR;
  }
  if(socket.isOpen())
  {
    socket.abort();
    socket.close();
  }
  socket.disconnectFromHost();
}

bool MainWindow::dev_connect(QString host,int _port)
{
    dev_disconnect();
    socket.connectToHost(host, _port);
    socket.waitForConnected(1000);

    if (!socket.isOpen()) return false;

    session = libssh2_session_init();

    if (session)
    {
        libssh2_session_set_blocking(session, 1);
        rc = libssh2_session_startup(session, socket.socketDescriptor());

        if (libssh2_userauth_password(session, edUser->text().toLocal8Bit(), edPassword->text().toLocal8Bit()) == 0)
        {

            sftp_session = libssh2_sftp_init(session);
            rawCfg = sftpFileRead(sftp_session, sftpfile);
            config.setDataSource(&rawCfg);

            cbDev->clear();
            QStringList tty = sftpListDir(sftp_session, "/dev");
            foreach (QString t, tty)
            {
                bool num = false;
                t.mid(3,t.length()-3).toUInt(&num);
                if ((t.left(3) == "tty") && !num && (t.length() > 3))
                    cbDev->addItem(t);
            }
            vlogPath.clear();
            if (config.value("VLOG/Enabled").toBool())
            {
                    vlogPath = config.value("VLOG/MountPoint").toString()+"/pi/";
                    vlogPath += config.value("VLOG/Folder").toString();
            }

            libssh2_session_callback_set(session,LIBSSH2_CALLBACK_DISCONNECT,(void*)&disconnect_callback);
            fillInterface();
            return true;
        }
    }
  return false;
}

bool MainWindow::is_connected()
{
 return (sftp_session != Q_NULLPTR) ? true : false;
}

void MainWindow::on_btConnect_clicked()
{
    if(dev_connect(edHost->currentText(), port))
    {
        enable_connect_controls(true);
        if(0>edHost->findText(edHost->currentText()))
             edHost->addItem(edHost->currentText());

    }else
     enable_connect_controls(false);



}

void MainWindow::on_btDownload_clicked()
{
    int row = lwCamNames->currentIndex().row();
    if (row < 0) return;
    QString camName = camList.item(row, COL_NAME)->text();
    QString camPath = vlogPath+"/"+camName;
    DownloadDialog dlg(&session);
    dlg.setPath(camPath);
    dlg.setWindowTitle(QString(tr("Video fixation %1")).arg(camName));
    dlg.exec();
    //qDebug() << Q_FUNC_INFO << camPath;
}

void MainWindow::on_pushButtonSave_clicked()
{
    config.setValue("DEV/ID",sbDevID->value());
    config.setValue("USART/BAUD",sbUARTBaud->value());
    config.setValue("USART/DEVICE",cbDev->currentText());


    config.push();
    //qDebug() << rawCfg;

    LIBSSH2_SFTP_HANDLE* handle = libssh2_sftp_open(sftp_session, sftpfile.toLocal8Bit(), LIBSSH2_FXF_CREAT | LIBSSH2_FXF_TRUNC | LIBSSH2_FXF_WRITE, 0);
    libssh2_sftp_write(handle, rawCfg.constData(), rawCfg.count());
    libssh2_sftp_close(handle);
    pushButtonSave->setEnabled(false);
    pushButtonReject->setEnabled(false);
}



void MainWindow::on_btDel_clicked()
{
    int row = lwCamNames->currentIndex().row();
    if (row < 0) return;
    if (QMessageBox::question(this, tr("Delete"), QString(tr("Delete camera \"%1\"?")).arg(camList.item(row, COL_NAME)->text()), QMessageBox::Yes | QMessageBox::No ) == QMessageBox::Yes)
    {
        config.delGroup(camList.item(row, COL_SYS_NAME)->text());
        camList.removeRow(row);
        onCamDataChanged();
    }
}

void MainWindow::on_btAdd_clicked()
{
    camAddDialog dlg;

    //prepare data
    QStringList namesList;
    QStringList sysNamesList;
    QList<quint8> idList;

    for (int i = 0; i < camList.rowCount(); i++)
    {
        namesList.append(camList.item(i, COL_NAME)->text());
        sysNamesList.append(camList.item(i, COL_SYS_NAME)->text());
        idList.append((quint8)camList.item(i, COL_ID)->text().toUInt());
    }

    QStringList lst = config.value("DEV/LIST").toString().split(",");
    dlg.setExistsData(namesList, sysNamesList, idList);

    if (dlg.exec() == QDialog::Accepted)
    {
        lst.append(dlg.edSysName->text());
        config.setValue("DEV/LIST", lst.join(","));
        config.setValue(QString("%1/ID").arg(dlg.edSysName->text()), dlg.sbID->value());
        config.setValue(QString("%1/MRL").arg(dlg.edSysName->text()), dlg.edMRL->text());
        config.setValue(QString("%1/Name").arg(dlg.edSysName->text()), dlg.edName->text());
        config.setValue(QString("%1/DISPLAY").arg(dlg.edSysName->text()), true);

        fillInterface();
        onChanged();
    }
}

void MainWindow::on_edHost_currentTextChanged(const QString &arg1)
{
  //Изменили адрес
    bool enabled = !arg1.isEmpty();
    btConnect->setEnabled(enabled);
    edUser->setEnabled(enabled);
    edPassword->setEnabled(enabled);
}



bool MainWindow::write_update_command()
{
    if(sftp_session )
    {
        const char * upd_cmd_file = "/home/pi/bin/.apply_update";
        LIBSSH2_SFTP_HANDLE * f_handle =
        libssh2_sftp_open_ex(sftp_session
                             ,upd_cmd_file,strlen(upd_cmd_file)
                             ,LIBSSH2_FXF_WRITE|LIBSSH2_FXF_CREAT|LIBSSH2_FXF_TRUNC
                             ,LIBSSH2_SFTP_S_IRUSR | LIBSSH2_SFTP_S_IWUSR | LIBSSH2_SFTP_S_IRGRP | LIBSSH2_SFTP_S_IWGRP | LIBSSH2_SFTP_S_IROTH | LIBSSH2_SFTP_S_IWOTH
                             ,LIBSSH2_SFTP_OPENFILE);
       if(f_handle)
       {
           const char * cmd = "Execute Update file";
           libssh2_sftp_write(f_handle,cmd,strlen(cmd));
           return 0 == libssh2_sftp_close_handle(f_handle) ? true : false;
       }
    }
 return false;
}

bool MainWindow::write_update_file(const QString & src_name, const QString & dest_name)
{
  if(sftp_session )
  {
      QFile src_file(src_name);
      if(src_file.exists() && src_file.open(QIODevice::ReadOnly) && !dest_name.isEmpty())
      {
        LIBSSH2_SFTP_HANDLE * f_handle =
        libssh2_sftp_open_ex(sftp_session
                             ,dest_name.toLocal8Bit().constData(),dest_name.length()
                             ,LIBSSH2_FXF_WRITE|LIBSSH2_FXF_CREAT|LIBSSH2_FXF_TRUNC
                             ,LIBSSH2_SFTP_S_IRWXU | LIBSSH2_SFTP_S_IRWXG | LIBSSH2_SFTP_S_IRWXO
                             ,LIBSSH2_SFTP_OPENFILE);
        if(f_handle)
        {
            std::vector<char> _buff;
            _buff.resize(16384,0);
            char * buf = &_buff.front();
            int64_t rd_sz =  src_file.read(buf,_buff.size());
            bool success = true;
            while(rd_sz && success)
            {
                success = (rd_sz == libssh2_sftp_write(f_handle,buf,rd_sz)) ? true : false;
                rd_sz =  src_file.read(buf,_buff.size());
                qApp->processEvents();

            }

             return (success &&  (0 == libssh2_sftp_close_handle(f_handle))) ? true : false ;
        }
      }
  }
  return false;
}


void MainWindow::on_btUpdate_clicked()
{
  //Обновление программы  записи на Raspberry pi
    QString fileName = QFileDialog::getOpenFileName(this,
                                tr("Open the update file"),
                                QDir::currentPath(),
                                tr("All files (*.*)")
                                                    );
    if(!fileName.isEmpty() && write_update_file(fileName,QString("/home/pi/bin/vhub.update")))
    {
      if(write_update_command())
       {
           dev_disconnect();
           QMessageBox::information(this, title,tr("The update was successful. Reconnect"));
       }

    }

}

void MainWindow::on_actionEN_triggered()
{
    actionRU->setChecked(false);
    actionEN->setChecked(true);
    qtLanguageTranslator.load(QString("QtLanguage_") + QString("en_EN"), qApp->applicationDirPath());
    qApp->installTranslator(&qtLanguageTranslator);
}

void MainWindow::on_actionRU_triggered()
{
    actionEN->setChecked(false);
    actionRU->setChecked(true);
    qtLanguageTranslator.load(QString("QtLanguage_") + QString("ru_RU"), qApp->applicationDirPath());
    qApp->installTranslator(&qtLanguageTranslator);
}
