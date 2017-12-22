#-------------------------------------------------
#
# Project created by QtCreator 2015-05-01T18:06:39
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = logic
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    rice_sample.cpp \
    guohall.cpp \
    zhangsuen.cpp \
    rice_grain.cpp \
    segmentation.cpp \
    tracker.cpp \
    support.cpp

INCLUDEPATH += /usr/local/include

LIBS += -L/usr/local/lib \
            -lopencv_core \
            -lopencv_imgproc \
            -lopencv_imgcodecs \
            -lopencv_highgui


HEADERS += \
    rice_sample.h \
    imshow.h \
    guohall.h \
    zhangsuen.h \
    rice_grain.h \
    segmentation.h \
    tracker.h \
    support.h
