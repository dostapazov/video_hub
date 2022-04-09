#-------------------------------------------------
#
# Project created by QtCreator 2018-05-28T11:29:05
#
#-------------------------------------------------

QT       += core gui serialport network
CONFIG   += thread c++17

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = vhub
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

#DEFINES += USE_NATIVE_PLAYER_WINDOW

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

win32:{
 INCLUDEPATH += "C:\Program Files\VideoLAN\VLC\sdk\include"
 LIBS += -L"C:\Program Files\VideoLAN\VLC\sdk\lib"
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

SOURCES +=\
	cam_logger.cpp \
	fan_control.cpp \
	main.cpp \
		mainwindow.cpp \
	appconfig.cpp \
	crc.cpp \
	proto.cpp \
	applog.cpp \
	filedeleterthread.cpp \
	recvparser.cpp \
	uart_comm.cpp \
	vlclasses.cpp

HEADERS  += mainwindow.h \
	appconfig.h \
	cam_logger.h \
	crc.h \
	proto.h \
	applog.h \
	filedeleterthread.h \
	recvparser.hpp \
	vlcclasses.hpp

FORMS    += mainwindow.ui
