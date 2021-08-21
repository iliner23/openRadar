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
greaterThan(QT_MAJOR_VERSION, 5): DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    author.cpp \
    commonfunctions.cpp \
    customscene.cpp \
    delegate.cpp \
    keychoose.cpp \
    keyreader.cpp \
    keysremedlist.cpp \
    label.cpp \
    labelrender.cpp \
    main.cpp \
    mainwindow.cpp \
    openctree.cpp \
    proxysearchmodel.cpp \
    remed_author.cpp \
    repchose.cpp \
    repertory.cpp \
    repertorydata.cpp \
    repertoryrender.cpp \
    searchmodel.cpp \
    searchresult.cpp \
    vocabulary.cpp \
    windowchapters.cpp

HEADERS += \
    author.h \
    cache.h \
    commonfunctions.h \
    customscene.h \
    delegate.h \
    keychoose.h \
    keyreader.h \
    keysremedlist.h \
    label.h \
    labelrender.h \
    languages.h \
    mainwindow.h \
    openctree.h \
    proxysearchmodel.h \
    remed_author.h \
    repchose.h \
    repertory.h \
    repertorydata.h \
    repertoryrender.h \
    searchmodel.h \
    searchresult.h \
    vocabulary.h \
    windowchapters.h

FORMS += \
    author.ui \
    keychoose.ui \
    keyreader.ui \
    keysremedlist.ui \
    label.ui \
    mainwindow.ui \
    remed_author.ui \
    repchose.ui \
    searchresult.ui \
    vocabulary.ui \
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
