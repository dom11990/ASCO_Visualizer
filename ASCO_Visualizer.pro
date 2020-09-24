#-------------------------------------------------
#
# Project created by QtCreator 2020-09-16T18:15:39
#
#-------------------------------------------------

QT       += core gui concurrent network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ASCO_Visualizer
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



INCLUDEPATH +="/usr/include/qwt"
INCLUDEPATH +="src"

CONFIG += qwt
LIBS += -L/usr/lib -lqwt

SOURCES += \
    src/asco_parameter.cpp \
    src/displayer.cpp \
    src/main.cpp \
    src/mainwindow.cpp

HEADERS += \
    src/asco_parameter.h \
    src/asco_parameter.hpp \
    src/displayer.h \
    src/mainwindow.h

FORMS += \
        src/mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
