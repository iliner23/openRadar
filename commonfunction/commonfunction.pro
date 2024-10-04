QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat

TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

Debug{
    LIBS += -L$$PWD/../build/openCtree/debug -lopenCtree

    win32{
        PRE_TARGETDEPS += $$PWD/../build/openCtree/debug/openCtree.lib
    }
}
Release{
    LIBS += -L$$PWD/../build/openCtree/release -lopenCtree

    win32{
        PRE_TARGETDEPS += $$PWD/../build/openCtree/release/openCtree.lib
    }
}

INCLUDEPATH += $$PWD/../openCtree

SOURCES += \
    commonfunctions.cpp \
    customscene.cpp \
    repertorydata.cpp \
    repertoryrender.cpp \
    widgetaction.cpp

HEADERS += \
    commonfunctions.h \
    customscene.h \
    languages.h \
    repertorydata.h \
    repertoryrender.h \
    widgetaction.h
