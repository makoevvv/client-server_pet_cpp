QT += core widgets network

CONFIG += c++17

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

LIBS += -L../build/common -lcommon

INCLUDEPATH += ../common

DESTDIR = ../build/qt-client