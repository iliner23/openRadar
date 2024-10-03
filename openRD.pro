QT += core gui multimedia

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
    author_info/author.cpp \
    chapters/delegate.cpp \
    chapters/proxysearchmodel.cpp \
    chapters/searchmodel.cpp \
    chapters/windowchapters.cpp \
    clipboards/enabledmeasure.cpp \
    clipboards/remediessortclass.cpp \
    clipboards/researchremed.cpp \
    clipboards/researchremedrender.cpp \
    clipboards/setremedy.cpp \
    clipboards/takeremed.cpp \
    commonfunction/commonfunctions.cpp \
    commonfunction/customscene.cpp \
    commonfunction/repertorydata.cpp \
    commonfunction/repertoryrender.cpp \
    commonfunction/widgetaction.cpp \
    keys/keychoose.cpp \
    keys/keyreader.cpp \
    keys/keysremedlist.cpp \
    main.cpp \
    mainwindow.cpp \
    openctree/openctree.cpp \
    remed_info/photoviewer.cpp \
    remed_info/remed_author.cpp \
    repertory/repchose.cpp \
    repertory/repertory.cpp \
    symptom_info/label.cpp \
    symptom_info/labelrender.cpp \
    symptom_search/searchresult.cpp \
    symptom_search/vocabulary.cpp
HEADERS += \
    author_info/author.h \
    chapters/delegate.h \
    chapters/proxysearchmodel.h \
    chapters/searchmodel.h \
    chapters/windowchapters.h \
    clipboards/enabledmeasure.h \
    clipboards/remediessortclass.h \
    clipboards/researchremed.h \
    clipboards/researchremedrender.h \
    clipboards/setremedy.h \
    clipboards/takeremed.h \
    commonfunction/commonfunctions.h \
    commonfunction/customscene.h \
    commonfunction/languages.h \
    commonfunction/repertorydata.h \
    commonfunction/repertoryrender.h \
    commonfunction/widgetaction.h \
    keys/keychoose.h \
    keys/keyreader.h \
    keys/keysremedlist.h \
    mainwindow.h \
    openctree/openctree.h \
    remed_info/photoviewer.h \
    remed_info/remed_author.h \
    repertory/repchose.h \
    repertory/repertory.h \
    symptom_info/label.h \
    symptom_info/labelrender.h \
    symptom_search/searchresult.h \
    symptom_search/vocabulary.h
FORMS += \
    author_info/author.ui \
    chapters/windowchapters.ui \
    clipboards/enabledmeasure.ui \
    clipboards/researchremed.ui \
    clipboards/setremedy.ui \
    clipboards/takeremed.ui \
    keys/keychoose.ui \
    keys/keyreader.ui \
    keys/keysremedlist.ui \
    mainwindow.ui \
    remed_info/photoviewer.ui \
    remed_info/remed_author.ui \
    repertory/repchose.ui \
    symptom_info/label.ui \
    symptom_search/searchresult.ui \
    symptom_search/vocabulary.ui

TRANSLATIONS += \
    openRD_ru_RU.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

DISTFILES += \
    README.md
