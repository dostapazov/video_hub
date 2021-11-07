/*
 vlc media
 vlc player interface class interfaces
 Ostapenko D. V. 2018-11-06 NIKTES
*/


#ifndef VLCCLASSES_H
#define VLCCLASSES_H

#include <vlc/vlc.h>
#include <qobject.h>

Q_DECLARE_METATYPE(libvlc_event_t);

namespace vlc {


class vlc_instance
{
    explicit   vlc_instance(const vlc_instance& /*src*/)   {}
    explicit   vlc_instance(const vlc_instance* /*src*/)   {}
    vlc_instance& operator = (const vlc_instance& /*src*/) {return *this;}

public :

    vlc_instance();
    vlc_instance(const int argc, const char* const argv[]);
    ~vlc_instance();


    static const vlc_instance* get_instance(const int argc, const char* const argv[] );
    static const vlc_instance* get_instance();
    static libvlc_instance_t*   create_instance(const int argc = 0, const char* const argv [] = nullptr);
    static              void    release_instance(libvlc_instance_t*   inst  );
    static      const char*     get_version();


    const libvlc_instance_t* operator()()const { return m_instance;}

private:
    libvlc_instance_t*     m_instance;
    static vlc_instance*   vlc_inst;
};


class vlc_media : public QObject
{
    Q_OBJECT

public :
    virtual  ~vlc_media() {close();}
    vlc_media(QObject* parent = Q_NULLPTR);
    vlc_media(const libvlc_media_t*, QObject* parent = Q_NULLPTR);
    vlc_media(const vlc_media& vlc_media );
    vlc_media& operator = (const vlc_media& src );
    vlc_media& operator = (const libvlc_media_t* src );


    bool     open_location(const char* uri, const vlc_instance* inst = nullptr);
    bool     open_file    (const char* fname, const vlc_instance* inst = nullptr);
    bool     close();
    const libvlc_media_t*     operator ()()const {return m_media;}
    void     add_option(const char* option);
    void     add_option(const char* option, unsigned int flags);
    const char* get_mrl   ();
    libvlc_state_t get_state ();
    bool get_stats(libvlc_media_stats_t* p_stats );
    libvlc_time_t get_duration();
    bool          event_activate(libvlc_event_e event_type, bool activate );
    void          events_activate_all(bool active);
    void*         user_data();
    void          set_user_data(void* data);

Q_SIGNALS:
    void              media_event   (const libvlc_event_t event) ;
private:
    static void     event_callback( const libvlc_event_t*, void* user_data);
    void     handle_event  (const libvlc_event_t& event)  ;
    libvlc_media_t* m_media = nullptr;
};

class vlc_player : public QObject
{
    Q_OBJECT
    vlc_player( const vlc_player& src): QObject(src.parent()) {}

public:

    explicit vlc_player(const vlc_instance* inst = nullptr, QObject* parent = nullptr);
    virtual ~vlc_player();
    vlc_media* set_media(vlc_media* media);
    bool     event_activate(libvlc_event_e event_type, bool active);
    void     events_activate_all(bool active);
    bool     is_playing();
    bool     play();
    bool     pause();
    bool     stop(int timeout = 50);

    bool     has_media() {return m_current_media ? true : false;}
    bool     is_fullscreen();
    void     set_fullscreen(bool full);
    QStringList    get_last_errors();
    libvlc_state_t get_state ();

#ifdef __linux__
    void     set_drawable(uint32_t x11drawable);
    uint32_t get_drawable();
#endif
#if defined (_WIN32 ) || defined (_W64)
    void     set_drawable(void* drawable);
    void*    get_drawable();
#endif

Q_SIGNALS:
    void player_event(const libvlc_event_t  event);

private:
    libvlc_media_player_t* m_player = nullptr;
    vlc_media*              m_current_media   = nullptr;
    QStringList           last_errors;
    void           handle_event  (const libvlc_event_t& event)  ;
    static void           event_callback( const libvlc_event_t*, void* user_data);
};

// inlines
inline const char* vlc_media::get_mrl ()
{
    return m_media ? libvlc_media_get_mrl(m_media) : nullptr;
}

inline void     vlc_media::add_option(const char* option)
{
    if (m_media)
        libvlc_media_add_option(m_media, option) ;
}

inline void     vlc_media::add_option(const char* option, unsigned int flags)
{
    if (m_media)
        libvlc_media_add_option_flag(m_media, option, flags) ;
}

inline libvlc_state_t vlc_media::get_state()
{
    return  m_media ? libvlc_media_get_state(m_media)  : libvlc_Error;
}

inline bool vlc_media::get_stats(libvlc_media_stats_t* p_stats )
{
    return  (m_media && p_stats && libvlc_media_get_stats( m_media, p_stats)) ? true : false;
}

inline libvlc_time_t vlc_media::get_duration()
{
    return  m_media ?  libvlc_media_get_duration( m_media ) : libvlc_time_t(0);
}

inline void*         vlc_media::user_data    ()
{
    return m_media ? libvlc_media_get_user_data(m_media) : nullptr;
}

inline void          vlc_media::set_user_data(void* data)
{
    if (m_media)
        libvlc_media_set_user_data(m_media, data);
}

inline bool     vlc_player::is_playing()
{
    return libvlc_media_player_is_playing(m_player);
}

inline bool     vlc_player::play()
{
    if (!m_player)
        return false;
    libvlc_media_player_play(m_player);
    return true;
}

inline bool     vlc_player::pause()
{
    if (!m_player)
        return false;

    libvlc_media_player_pause(m_player);
    return true;
}

inline bool     vlc_player::is_fullscreen()
{
    return (this->m_player && libvlc_get_fullscreen(m_player) ) ? true : false;
}

inline void     vlc_player::set_fullscreen(bool full)
{
    if (m_player)
        libvlc_set_fullscreen(m_player, full ? 1 : 0);
}

inline QStringList vlc_player::get_last_errors()
{
    QStringList  ret = this->last_errors;
    last_errors.clear();
    return ret;
}

inline libvlc_state_t vlc_player::get_state ()
{
    return m_player ? libvlc_media_player_get_state(m_player) : libvlc_Error;
}

#ifdef __linux__
inline void     vlc_player::set_drawable(uint32_t x11drawable)
{
    if (m_player)
        libvlc_media_player_set_xwindow(m_player, x11drawable);
}

inline uint32_t vlc_player::get_drawable()
{
    return (m_player) ? libvlc_media_player_get_xwindow(m_player) : 0;
}
#endif

#if defined (_WIN32 ) || defined (_W64)
inline  void     vlc_player::set_drawable(void* drawable)
{
    Q_UNUSED(drawable);
}

inline  void*    vlc_player::get_drawable()
{
    return nullptr;
}
#endif

}

#endif // VLCCLASSES_H

