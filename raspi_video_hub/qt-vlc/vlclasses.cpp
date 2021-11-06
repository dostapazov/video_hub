/*
 vlc media
 vlc player interface class implemantation
 Ostapenko D. V. 2018-11-06 NIKTES
*/

#include "vlcclasses.hpp"
#include <vlc/libvlc_media.h>
#include <qthread.h>
#include <qdebug.h>

namespace vlc {


vlc_instance*   vlc_instance::vlc_inst = nullptr;
vlc_instance::vlc_instance()
{
    m_instance = create_instance();
}

vlc_instance::vlc_instance(const int argc, const char* const argv[])
{
    m_instance = create_instance(argc, argv);
}

vlc_instance::~vlc_instance()
{
    release_instance(this->m_instance);
}

const char*    vlc_instance::get_version()
{
    return libvlc_get_version();
}


const vlc_instance* vlc_instance::get_instance(const int argc, const char*   const argv[] )
{
    if (!vlc_inst)
        vlc_inst = new vlc_instance(argc, argv);
    return vlc_inst;
}

const vlc_instance* vlc_instance::get_instance()
{
    if (!vlc_inst)
        vlc_inst = new vlc_instance;
    return vlc_inst;
}

libvlc_instance_t*   vlc_instance::create_instance(const int argc, const char* const argv[] )
{

    qRegisterMetaType<libvlc_event_t>("libvlc_event_t");
    return libvlc_new(argc, argv);
}

void    vlc_instance::release_instance(libvlc_instance_t*   inst  )
{
    if (inst)
        libvlc_release(inst);
}



vlc_media::vlc_media(QObject* parent): QObject(parent)
{}

vlc_media::vlc_media(const libvlc_media_t* src, QObject* parent): QObject(parent)
{
    *this = src;
}

vlc_media::vlc_media(const vlc_media& src  ): QObject(src.parent())
{
    *this = src;
}

vlc_media& vlc_media::operator = (const vlc_media& src )
{
    close();
    *this = src();
    return *this;
}

vlc_media& vlc_media::operator = (const libvlc_media_t* src )
{
    close();
    m_media  = const_cast<libvlc_media_t*>(src);
    libvlc_media_retain(m_media);
    return *this;
}


bool     vlc_media::open_location(const char* mrl, const vlc_instance* inst)
{
    close();
    libvlc_instance_t* i = const_cast<libvlc_instance_t*>( inst ? (*inst)() : (*vlc_instance::get_instance())() );
    m_media  = libvlc_media_new_location(i, mrl);
    return m_media ? true : false;
}

bool     vlc_media::open_file    (const char* fname, const vlc_instance* inst)
{
    close();
    libvlc_instance_t* i = const_cast<libvlc_instance_t*>( inst ? (*inst)() : (*vlc_instance::get_instance())() );
    m_media  = libvlc_media_new_path(i, fname);
    return m_media ? true : false;
}

bool     vlc_media::close()
{
    if (this->m_media)
    {
        libvlc_media_release(m_media);
        m_media = nullptr;
        return  true;
    }
    return false;
}

bool    vlc_media::event_activate(libvlc_event_e event_type, bool active )
{
    if (m_media)
    {
        libvlc_event_manager_t* mng = libvlc_media_event_manager(m_media);
        if (mng)
        {
            if (active)
            {
                return libvlc_event_attach(mng, event_type, event_callback, this) ? false : true;
            }
            else
            {
                libvlc_event_detach(mng, event_type, event_callback, this);
                return true;
            }
        }
    }
    return false;
}

void    vlc_media::events_activate_all(bool active)
{
    if (m_media )
    {
        libvlc_event_manager_t* mng = libvlc_media_event_manager(m_media);
        if (mng)
        {
            if (active)
            {
                libvlc_event_attach(mng, libvlc_MediaMetaChanged, event_callback, this);
                libvlc_event_attach(mng, libvlc_MediaSubItemAdded, event_callback, this);
                libvlc_event_attach(mng, libvlc_MediaDurationChanged, event_callback, this);
                libvlc_event_attach(mng, libvlc_MediaParsedChanged, event_callback, this);
                libvlc_event_attach(mng, libvlc_MediaFreed, event_callback, this);
                libvlc_event_attach(mng, libvlc_MediaStateChanged, event_callback, this);
                libvlc_event_attach(mng, libvlc_MediaSubItemTreeAdded, event_callback, this);
            }
            else
            {
                libvlc_event_detach(mng, libvlc_MediaMetaChanged, event_callback, this);
                libvlc_event_detach(mng, libvlc_MediaSubItemAdded, event_callback, this);
                libvlc_event_detach(mng, libvlc_MediaDurationChanged, event_callback, this);
                libvlc_event_detach(mng, libvlc_MediaParsedChanged, event_callback, this);
                libvlc_event_detach(mng, libvlc_MediaFreed, event_callback, this);
                libvlc_event_detach(mng, libvlc_MediaStateChanged, event_callback, this);
                libvlc_event_detach(mng, libvlc_MediaSubItemTreeAdded, event_callback, this);
            }

        }
    }
}


void     vlc_media::handle_event  (const libvlc_event_t& event)
{
    emit media_event(event);
}

void    vlc_media::event_callback( const libvlc_event_t* _event, void* user_data)
{
    vlc_media* media = reinterpret_cast<vlc_media*>(user_data);
    if (media && _event)
    {
        libvlc_event_t event = *_event;
        media->handle_event(event);
    }
}

vlc_player::vlc_player(const vlc_instance* inst, QObject* parent): QObject(parent)
{
    if (!inst)
        inst = vlc_instance::get_instance();
    m_player = libvlc_media_player_new(const_cast<libvlc_instance_t*>((*inst)()));
}

vlc_player::~vlc_player()
{
    set_media(nullptr);
    if (m_player)
        libvlc_media_player_release(m_player);

}

bool     vlc_player::stop(int timeout )
{
    if (m_player)
    {
        libvlc_media_player_stop(m_player);
        if (timeout)
        {
            QThread::msleep(timeout);
        }
        return true;
    }
    return false;
}


vlc_media* vlc_player::set_media(vlc_media* media)
{
    if (m_current_media != media)
    {
//        bool _playing = this->is_playing();
//        if (_playing)
//            stop();
        libvlc_media_player_set_media(m_player, media ? const_cast<libvlc_media_t*>((*media)()) : nullptr);
        vlc_media* old_media = m_current_media;
        m_current_media = media;
        if (media && !is_playing())
            play();
        return old_media;
    }
    return nullptr;


}

bool     vlc_player::event_activate(libvlc_event_e event_type, bool active)
{
    if (m_player )
    {
        libvlc_event_manager_t* mng = libvlc_media_player_event_manager(m_player);
        if (mng)
        {
            if (active)
                return  libvlc_event_attach(mng, event_type, event_callback, this) ? false : true ;
            else
            {
                libvlc_event_detach       (mng, event_type, event_callback, this);
                return true;
            }
        }
    }
    return false;
}

void     vlc_player::events_activate_all(bool active)
{
    if (m_player  )
    {
        libvlc_event_manager_t* mng = libvlc_media_player_event_manager(m_player);
        if (!mng)
            return;

        if (active)
        {
            libvlc_event_attach(mng, libvlc_MediaPlayerMediaChanged, event_callback, this);
            libvlc_event_attach(mng, libvlc_MediaPlayerNothingSpecial, event_callback, this);
            libvlc_event_attach(mng, libvlc_MediaPlayerOpening, event_callback, this);
            libvlc_event_attach(mng, libvlc_MediaPlayerBuffering, event_callback, this);
            libvlc_event_attach(mng, libvlc_MediaPlayerPlaying, event_callback, this);
            libvlc_event_attach(mng, libvlc_MediaPlayerPaused, event_callback, this);
            libvlc_event_attach(mng, libvlc_MediaPlayerStopped, event_callback, this);
            libvlc_event_attach(mng, libvlc_MediaPlayerForward, event_callback, this);
            libvlc_event_attach(mng, libvlc_MediaPlayerBackward, event_callback, this);
            libvlc_event_attach(mng, libvlc_MediaPlayerEndReached, event_callback, this);
            libvlc_event_attach(mng, libvlc_MediaPlayerEncounteredError, event_callback, this);
            libvlc_event_attach(mng, libvlc_MediaPlayerTimeChanged, event_callback, this);
            libvlc_event_attach(mng, libvlc_MediaPlayerPositionChanged, event_callback, this);
            libvlc_event_attach(mng, libvlc_MediaPlayerSeekableChanged, event_callback, this);
            libvlc_event_attach(mng, libvlc_MediaPlayerPausableChanged, event_callback, this);
            libvlc_event_attach(mng, libvlc_MediaPlayerTitleChanged, event_callback, this);
            libvlc_event_attach(mng, libvlc_MediaPlayerSnapshotTaken, event_callback, this);
            libvlc_event_attach(mng, libvlc_MediaPlayerLengthChanged, event_callback, this);
            libvlc_event_attach(mng, libvlc_MediaPlayerVout, event_callback, this);
            libvlc_event_attach(mng, libvlc_MediaPlayerScrambledChanged, event_callback, this);
            libvlc_event_attach(mng, libvlc_MediaPlayerCorked, event_callback, this);
            libvlc_event_attach(mng, libvlc_MediaPlayerUncorked, event_callback, this);
            libvlc_event_attach(mng, libvlc_MediaPlayerMuted, event_callback, this);
            libvlc_event_attach(mng, libvlc_MediaPlayerUnmuted, event_callback, this);
            libvlc_event_attach(mng, libvlc_MediaPlayerAudioVolume, event_callback, this);
        }
        else
        {
            libvlc_event_detach(mng, libvlc_MediaPlayerMediaChanged, event_callback, this);
            libvlc_event_detach(mng, libvlc_MediaPlayerNothingSpecial, event_callback, this);
            libvlc_event_detach(mng, libvlc_MediaPlayerOpening, event_callback, this);
            libvlc_event_detach(mng, libvlc_MediaPlayerBuffering, event_callback, this);
            libvlc_event_detach(mng, libvlc_MediaPlayerPlaying, event_callback, this);
            libvlc_event_detach(mng, libvlc_MediaPlayerPaused, event_callback, this);
            libvlc_event_detach(mng, libvlc_MediaPlayerStopped, event_callback, this);
            libvlc_event_detach(mng, libvlc_MediaPlayerForward, event_callback, this);
            libvlc_event_detach(mng, libvlc_MediaPlayerBackward, event_callback, this);
            libvlc_event_detach(mng, libvlc_MediaPlayerEndReached, event_callback, this);
            libvlc_event_detach(mng, libvlc_MediaPlayerEncounteredError, event_callback, this);
            libvlc_event_detach(mng, libvlc_MediaPlayerTimeChanged, event_callback, this);
            libvlc_event_detach(mng, libvlc_MediaPlayerPositionChanged, event_callback, this);
            libvlc_event_detach(mng, libvlc_MediaPlayerSeekableChanged, event_callback, this);
            libvlc_event_detach(mng, libvlc_MediaPlayerPausableChanged, event_callback, this);
            libvlc_event_detach(mng, libvlc_MediaPlayerTitleChanged, event_callback, this);
            libvlc_event_detach(mng, libvlc_MediaPlayerSnapshotTaken, event_callback, this);
            libvlc_event_detach(mng, libvlc_MediaPlayerLengthChanged, event_callback, this);
            libvlc_event_detach(mng, libvlc_MediaPlayerVout, event_callback, this);
            libvlc_event_detach(mng, libvlc_MediaPlayerScrambledChanged, event_callback, this);
            libvlc_event_detach(mng, libvlc_MediaPlayerCorked, event_callback, this);
            libvlc_event_detach(mng, libvlc_MediaPlayerUncorked, event_callback, this);
            libvlc_event_detach(mng, libvlc_MediaPlayerMuted, event_callback, this);
            libvlc_event_detach(mng, libvlc_MediaPlayerUnmuted, event_callback, this);
            libvlc_event_detach(mng, libvlc_MediaPlayerAudioVolume, event_callback, this);
        }
    }
}


void    vlc_player::handle_event(const libvlc_event_t& event)
{
    if (event.type == libvlc_MediaPlayerEncounteredError )
        last_errors.append( QString(libvlc_errmsg()));

    emit  player_event(event);

}

void     vlc_player::event_callback( const  libvlc_event_t* _event, void* user_data)
{
    vlc_player* player = reinterpret_cast<vlc_player*>(user_data);
    if (player && _event)
    {
        libvlc_event_t event = *_event;
        player->handle_event(event);
    }
    else
        qDebug() << player->tr("Invalid event callback params player %1 , event %2")
                 .arg(player ? "not null" : "NULL")
                 .arg(_event ? "not null" : "NULL");
}

}// end of  namespace vlc


