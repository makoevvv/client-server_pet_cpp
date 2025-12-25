QT += core widgets network

CONFIG += c++17

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    clientinfo.cpp

HEADERS += \
    mainwindow.h \
    clientinfo.h

LIBS += -L../build/common -lcommon

INCLUDEPATH += ../common

DESTDIR = ../build/qt-server