QT += testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase c++17
CONFIG -= app_bundle

TEMPLATE = app
TARGET = preferencetest

#	https://stackoverflow.com/questions/37920115/
#	understanding-the-term-pwd-in-qmake-project-file
DEFINES += SRCDIR=\\\"$$PWD/\\\"

SOURCES +=	preferencetest.cc \
		../../PWAnalyzer/preference.cc \
		../../PWAnalyzer/jsonmanager.cc \


HEADERS += \
		../../PWAnalyzer/preference.hh \
		../../PWAnalyzer/jsonmanager.hh \

INCLUDEPATH += \
		../../PWAnalyzer \
