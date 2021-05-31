QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat

CONFIG += c++17

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x050F00

SOURCES += \
    author.cpp \
    customscene.cpp \
    delegate.cpp \
    label.cpp \
    labelengine.cpp \
    main.cpp \
    mainwindow.cpp \
    openctree.cpp \
    proxysearchmodel.cpp \
    remed_author.cpp \
    repchose.cpp \
    repertory.cpp \
    repertoryengine.cpp \
    searchmodel.cpp \
    windowchapters.cpp

HEADERS += \
    author.h \
    cache.h \
    customscene.h \
    delegate.h \
    label.h \
    labelengine.h \
    mainwindow.h \
    openctree.h \
    proxysearchmodel.h \
    remed_author.h \
    repchose.h \
    repertory.h \
    repertoryengine.h \
    searchmodel.h \
    windowchapters.h

FORMS += \
    author.ui \
    label.ui \
    mainwindow.ui \
    remed_author.ui \
    repchose.ui \
    windowchapters.ui

TRANSLATIONS += \
    openRD_ru_RU.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

DISTFILES +=
