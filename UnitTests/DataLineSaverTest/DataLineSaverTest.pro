QT += testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase c++17
CONFIG -= app_bundle

TEMPLATE = app
TARGET = datalinesavertest

#	https://stackoverflow.com/questions/37920115/
#	understanding-the-term-pwd-in-qmake-project-file
DEFINES += SRCDIR=\\\"$$PWD/\\\"

SOURCES +=	datalinesavertest.cc \
		../../PWAnalyzer/datalinesaver.cc \
		../../PWAnalyzer/jsonmanager.cc \
		../../PWAnalyzer/utils.cc \


HEADERS += \
		../../PWAnalyzer/datalinesaver.hh \
		../../PWAnalyzer/jsonmanager.hh \
		../../PWAnalyzer/utils.hh \

INCLUDEPATH += \
		../../PWAnalyzer \
