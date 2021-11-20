#-------------------------------------------------
#
# Project created by QtCreator 2018-08-20T08:52:04
#
#-------------------------------------------------

QT       += core gui widgets network

windows:{
#include(./common/libssh/libssh.pri)
}

TARGET = video_hub_cfg
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
		main.cpp \
		mainwindow.cpp \
	memoryini.cpp \
	downloaddialog.cpp \
	camadddialog.cpp

HEADERS += \
		mainwindow.h \
	memoryini.h \
	downloaddialog.h \
	camadddialog.h

FORMS += \
		mainwindow.ui \
	downloaddialog.ui \
	progressform.ui \
	camadddialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
	video_hub.qrc

linux:{
	LIBS += -l ssh2
}

windows:{
	HEADERS += fileinfo.h
	SOURCES += fileinfo.cpp
	RC_FILE = hub_config.rc
	LIBS += libversion libpsapi
}

TRANSLATIONS += QtLanguage_ru.ts
CODECFORSRC = UTF-8
