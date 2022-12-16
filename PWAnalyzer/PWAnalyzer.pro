QT       += core gui charts network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    apitypes.cc \
    calcscontroller.cc \
    calcswidget.cc \
    chartpresenter.cc \
    chartwidget.cc \
    datalinesmodel.cc \
    fingrid.cc \
    fmi.cc \
    main.cc \
    mainwindow.cc \
	jsonmanager.cc \
	datalinesaver.cc \
	preference.cc \
    webapi.cc \
    utils.cc

HEADERS += \
    apitypes.hh \
    calcscontroller.hh \
    calcswidget.hh \
    chartpresenter.hh \
    chartwidget.hh \
    datalinesmodel.hh \
    fingrid.hh \
    fmi.hh \
    iprovider.hh \
    mainwindow.hh \
 	jsonmanager.hh \
	datalinesaver.hh \
	preference.hh \
    webapi.hh \
    utils.hh

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
