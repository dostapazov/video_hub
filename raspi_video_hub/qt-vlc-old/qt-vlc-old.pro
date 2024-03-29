#-------------------------------------------------
#
# Project created by QtCreator 2018-05-28T11:29:05
#
#-------------------------------------------------

QT       += core gui serialport network
CONFIG   += thread c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = vhub-old
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

win32:{
 INCLUDEPATH += "C:\Program Files (x86)\VideoLAN\VLC\sdk\include"
 LIBS += -L"C:\Program Files (x86)\VideoLAN\VLC\sdk\lib"
 LIBS += -lvlc -lvlccore
}

unix:{
LIBS += -lvlc
#LIBS += -L/usr/X11/lib -lX11
!contains(DEFINES,DESKTOP_DEBUG_BUILD) :{
        CONFIG(release , debug | release) : {
                LIBS += -lwiringPi
        }
 }

}



SOURCES += recorder_main.cpp\
        mainwindow.cpp \
    appconfig.cpp \
    crc.cpp \
    proto.cpp \
    applog.cpp \
    camtimesync.cpp \
    filedeleterthread.cpp \
    vlclasses.cpp \
    main_window_uart.cpp \
    cam_logger_vlc.cpp

HEADERS  += mainwindow.h \
    appconfig.h \
    crc.h \
    proto.h \
    applog.h \
    camtimesync.h \
    filedeleterthread.h \
    vlcclasses.hpp \
    cam_logger_vlc.h

FORMS    += mainwindow.ui
