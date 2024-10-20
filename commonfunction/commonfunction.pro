QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat

TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

Debug {
    win32 {
        LIBS += -L$$PWD/../build/openCtree/debug -lopenCtree
        PRE_TARGETDEPS += $$PWD/../build/openCtree/debug/openCtree.lib
    }
    unix:linux {
        LIBS += -L$$PWD/../build/Debug/openCtree -lopenCtree
        PRE_TARGETDEPS += $$PWD/../build/Debug/openCtree/libopenCtree.a
    }
}
Release {
    win32 {
        LIBS += -L$$PWD/../build/openCtree/release -lopenCtree
        PRE_TARGETDEPS += $$PWD/../build/openCtree/release/openCtree.lib
    }
    unix:linux {
        LIBS += -L$$PWD/../build/Release/openCtree -lopenCtree
        PRE_TARGETDEPS += $$PWD/../build/Release/openCtree/libopenCtree.a
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
