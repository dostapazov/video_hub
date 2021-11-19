QT += testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

SOURCES +=  tst_test_parser.cpp \
	../qt-vlc/proto.cpp \
	../qt-vlc/recvparser.cpp

HEADERS += \
	../qt-vlc/proto.h \
	../qt-vlc/recvparser.hpp
