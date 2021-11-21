/*******************************************
 * Class CamTimeSync
 * designed for get time value from corresponding cameras
 * Implementation
 * Ostapenko D. V. 2018-10-30 NIKTES
 * ******************************************/

#include "camtimesync.h"
#include <qnetworkreply.h>
#include <qnetworkrequest.h>
#include <qnetworkcookie.h>
#include "applog.h"

CamTimeSync::CamTimeSync(QObject *parent) : QObject (parent)
           ,access_mgr(this)
           ,m_reply(Q_NULLPTR)

{
    m_timer.setSingleShot(true);
    connect(&m_timer,SIGNAL(timeout()),SLOT(timer_fired()));
}



bool CamTimeSync::start_sync(const QString &url_string,int interval)
{
    QUrl url(url_string);
    m_host = url.host();
    this->request_cam_time();
    schedule_next_request(interval);
    return m_reply != Q_NULLPTR;
}

void CamTimeSync::stop_sync ()
{
  this->m_timer.stop();
}

void CamTimeSync::schedule_next_request(int interval)
{
    m_timer.stop();
    m_timer.setInterval(interval);
    m_timer.start();
}

void CamTimeSync::timer_fired()
{
  if(m_reply )
  {
    if(!m_reply->isFinished())
    {
        close_reply();
        emit synchronized(false);

     }
    else
        this->schedule_next_request(m_timer.interval());
  }
  else
  {
    if(!request_cam_time() )
        emit synchronized(false);
  }

//  if((m_reply && !m_reply->isFinished()) || !request_cam_time() )
//  {
//    //QString str = tr(" reply timer fired %1").arg(this->m_timer.interval());
//    close_reply();
//    emit synchronized(false);
//  }
}

bool CamTimeSync::request_cam_time()
{
    if(!m_host.isEmpty())
    {
       close_reply();
       QUrl url;
       url.setScheme("http");
       url.setHost(m_host);
       url.setPath("/oneshotimage");
       QNetworkRequest req(url);
       m_reply = access_mgr.get(req);
       connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(replay_error(QNetworkReply::NetworkError)));
       connect(m_reply, SIGNAL(finished())                        , SLOT(replay_finished()));
       connect(m_reply, SIGNAL(metaDataChanged())                 , SLOT(replay_meta_changed()));
       return true;
    }
   return false;
}

void CamTimeSync::close_reply()
{
  if(m_reply)
  {
      QNetworkReply * reply = m_reply;
      reply->blockSignals(true);
      m_reply = Q_NULLPTR;
      if(reply->isRunning())
          reply->abort();
       reply->close();
       reply->deleteLater();
  }
}



void CamTimeSync::replay_finished()
{
  close_reply();
}

void CamTimeSync::replay_meta_changed()
{

  QByteArray hdr = QString("Date").toLocal8Bit();
  if(m_reply->hasRawHeader(hdr))
  {
   QByteArray hdr_data = m_reply->rawHeader(hdr);
   QString str = QString::fromLocal8Bit(hdr_data);
   m_date_time = QDateTime::fromString(str,Qt::RFC2822Date).toLocalTime();
   close_reply();

   qint64 diff = (m_date_time.toMSecsSinceEpoch() - QDateTime::currentDateTime().toMSecsSinceEpoch())/1000;
   if(abs(diff)>m_sec_delta)
       emit time_difference(m_date_time,diff);
       else
       emit synchronized(true);
  }
}

void CamTimeSync::replay_error(QNetworkReply::NetworkError err)
{
      Q_UNUSED(err);
      QString str = tr("%1 replay error %2").arg(this->m_host).arg(m_reply->errorString());
      qDebug()<<str;
      if(m_reply) emit synchronized(false);
      close_reply();
      emit synchronized(false);
}



