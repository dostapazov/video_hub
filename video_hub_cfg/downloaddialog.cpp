#include <QDebug>
#include <QFileDialog>

#include "downloaddialog.h"
#include <qmessagebox.h>

DownloadDialog::DownloadDialog(LIBSSH2_SESSION **sess, QWidget *parent) :
    QDialog(parent)
  , main_sess ( sess )
{
    // NOTE Для чего адрес адреса LIBSSH2_SESSION
    // Чтобы в дальнейшем установленный callback на LIBSSH2_DISCONNECT_FUNC
    // обнулял сессию
    setupUi(this);
    info_frame->setVisible(false);
}

DownloadDialog::~DownloadDialog()
{
    sftp_close();
}

bool DownloadDialog::sftp_init   ()
{
  if(this->main_sess && *main_sess)
      sftp_session = libssh2_sftp_init(*main_sess);
  return sftp_session ? true : false;
}

void DownloadDialog::sftp_close  ()
{
  if(sftp_session) libssh2_sftp_shutdown(sftp_session);
  sftp_session = Q_NULLPTR;

}


void DownloadDialog::setPath(QString path)
{
    rootPath = path;
    drawRootPath();
}

void DownloadDialog::drawRootPath()
{
    if(!sftp_session && !sftp_init()) return;
    char mem[512];
    char longentry[512];
    LIBSSH2_SFTP_ATTRIBUTES attrs;
    int res = 0;
    lastPath = rootPath;

    LIBSSH2_SFTP_HANDLE* handle = libssh2_sftp_opendir(sftp_session, rootPath.toLocal8Bit());
    lwFiles->clear();
    if (!handle) return;

    do
    {
        res = libssh2_sftp_readdir_ex(handle, mem, sizeof(mem),longentry, sizeof(longentry), &attrs);
        if (res)
        {
            //qDebug() << attrs.permissions << ((attrs.permissions & 0x4000) == 0) << longentry << res;
            if ( (attrs.permissions & 0x4000) && !(mem[0] == '.') )
            {
                QListWidgetItem* item = new QListWidgetItem();
                item->setText(QString(mem));
                item->setIcon(QIcon(":/Icons/FOLDER"));
                lwFiles->addItem(item);
            }
        }

    } while (res);
    libssh2_sftp_close(handle);
}

void DownloadDialog::changeEvent(QEvent *e)
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

void DownloadDialog::scan_folder(const QString & folder )
{
 //qDebug() << Q_FUNC_INFO << rootPath << item->text() << lastPath;
 lwFiles->clear();
 if(!sftp_session && !sftp_init()) return;
 LIBSSH2_SFTP_HANDLE* handle = libssh2_sftp_opendir(sftp_session, folder.toLocal8Bit());
 if(handle)
 {

     char mem[512];
     char longentry[512];
     LIBSSH2_SFTP_ATTRIBUTES attrs;

     int res = libssh2_sftp_readdir_ex(handle, mem, sizeof(mem),longentry, sizeof(longentry), &attrs);
    while (res)
    {
            //qDebug() << attrs.permissions << ((attrs.permissions & 0x4000) == 0) << longentry << res;
            if (res > 1)
            {
                QListWidgetItem* file = new QListWidgetItem(QString(mem));
                if ((((attrs.permissions & 0x4000) == 0) && (attrs.filesize > 0)) || (file->text() == ".."))
                {
                    if (file->text() != "..")
                    {
                        file->setData(Qt::UserRole, attrs.filesize);
                        file->setIcon(QIcon(":/Icons/VIDEO"));
                    }
                    else
                        file->setIcon(QIcon(":/Icons/LEFT_UP"));
                    lwFiles->addItem(file);
                }
            }
      res = libssh2_sftp_readdir_ex(handle, mem, sizeof(mem),longentry, sizeof(longentry), &attrs);
    };
    libssh2_sftp_close(handle);
   }

}

void DownloadDialog::on_lwFiles_itemDoubleClicked(QListWidgetItem *item)
{
    if (!item) return;
    if (item->data(Qt::UserRole).toLongLong() == 0) //change dir
    {
        if (item->text() == "..")
            drawRootPath();
        else
        {
           lastPath = rootPath+"/"+item->text();
           scan_folder(lastPath);
        }
    }
}



bool DownloadDialog::remove_file (const QString file_name)
{
    if(!sftp_session && !sftp_init()) return false;
    if(!file_name.isEmpty() && file_name != ".." )
    {
        std::string _file_name = file_name.toStdString();
        return 0 == libssh2_sftp_unlink_ex(this->sftp_session,_file_name.data(),file_name.length()) ? true : false;
    }
    return false;
}

int  DownloadDialog::remove_files(const QList<QListWidgetItem*> & files)
{
  int count = 0;
  foreach(const QListWidgetItem * item,files)
  {
     if(item->text() != "..")
     {
        QString file_name = lastPath + "/" + item->text();
        if(remove_file(file_name))
        {
            ++count;
        }

     }
 }
  return count;
}

void DownloadDialog::on_bRemoveFiles_clicked()
{
    //Удаление выбранных файлов
    auto sel_list = lwFiles->selectedItems();

    if(sel_list.count() &&
       QMessageBox::Yes == QMessageBox::question(this
                                                 , tr("Delete files")
                                                 , tr("Do you really want to delete the selected files?")
                                                 , QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No)
                                                 , QMessageBox::No
                                                 )
      )
    {
      remove_files(sel_list);
      scan_folder(lastPath);
    }
}



void DownloadDialog::on_lwFiles_itemSelectionChanged()
{
    auto sel_list = lwFiles->selectedItems();
    bool enabled  = (sel_list.count() && ( sel_list.count() >1  || !sel_list.at(0)->text().startsWith(".") ) ) ? true : false;
    bRemoveFiles->setEnabled(enabled);
    bCopy->setEnabled(enabled);
}

void DownloadDialog::on_bCopy_clicked()
{
    auto sel_list = lwFiles->selectedItems();
    if(sel_list.count())
        copy_files(sel_list);
}

void DownloadDialog::switch_frames()
{
 info_frame->setVisible(!info_frame->isVisible());
 btn_frame->setVisible(!info_frame->isVisible());
}

int  DownloadDialog::copy_files  ( const QList<QListWidgetItem*> & files)
{
 int copy_count = 0;
 QString dest_path = QFileDialog::getExistingDirectory(this, tr("Saving path"),"");
 if(!dest_path.isEmpty())
 {
  setEnabled(false);
  switch_frames();
    foreach(const QListWidgetItem * item,files)
    {
        if(!item->text().startsWith(".") )
        {
          QString src_name  = lastPath  + "/" + item->text();
          QString dest_name = dest_path + "/" + item->text();
          this->curr_file->setText(item->text()) ;
          copy_count += copy_file(src_name,dest_name,pbProgress);


        }
    }
    setEnabled(true);
    switch_frames();
    info_frame->setVisible(false);
  }
 return copy_count;
}

bool DownloadDialog::copy_file   (const QString& src_name, const QString& dst_name,QProgressBar * pb)
{
        if(!sftp_session && !sftp_init()) return false;

        QFile dstFile(dst_name);
        LIBSSH2_SFTP_HANDLE* handle;
        std::vector<char> mem;
        mem.resize(8192<<2,0);
        char * mem_ptr = &mem.at(0);
        int res = 0;
        if(!dstFile.open(QFile::WriteOnly)) return false;

        handle = libssh2_sftp_open(sftp_session, src_name.toLocal8Bit(), LIBSSH2_FXF_READ, 0);
        if(!handle) return false;
        if(pb)
        {
          pb->setMaximum(lwFiles->currentItem()->data(Qt::UserRole).toInt());
          pb->setValue(0);
        }


            int pass = 0;

            res = libssh2_sftp_read(handle, mem_ptr, mem.size());
            while(res)
            {
               dstFile.write(mem_ptr, res);
               dstFile.waitForBytesWritten(300);
               if(pb)
               {
                 pb->setValue(pb->value()+res);
                 if (!(++pass%10)) qApp->processEvents();
               }
              res = libssh2_sftp_read(handle, mem_ptr, mem.size());
            }while(res);

           libssh2_sftp_close_handle(handle);
           dstFile.close();
    return true;
}

