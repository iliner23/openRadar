#ifndef LANGUAGES_H
#define LANGUAGES_H
#include <QLocale>
#include <QTextCodec>
#include <array>

namespace lang {
    const std::array<QLocale, 13> radarLang = {
        QLocale::English,
        QLocale::French,
        QLocale::Italian,
        QLocale::Spanish,
        QLocale::German,
        QLocale::Dutch,
        QLocale::Portuguese,
        QLocale::Catalan,
        QLocale::Czech,
        QLocale::Russian,
        QLocale::Romanian,
        QLocale::Japanese,
        QLocale::Chinese
    };

    inline QByteArray langToName(QLocale lang){
       switch (lang.language()) {
       case QLocale::English:
       case QLocale::French:
       case QLocale::Italian:
       case QLocale::Spanish:
       case QLocale::Dutch:
       case QLocale::Portuguese:
       case QLocale::Catalan:
       case QLocale::German:
           return "windows-1252";
       case QLocale::Romanian:
       case QLocale::Czech:
           return "windows-1250";
       case QLocale::Russian:
           return "windows-1251";
       case QLocale::Japanese:
           return "Shift-JIS";
       case QLocale::Chinese:
           return "GB18030";
       default:
           return "windows-1252";
       }
    }
    inline QByteArray langToName(quint8 pos){
        return langToName(radarLang.at(pos));
    }

    inline QByteArray defaultCodec(){
        return "windows-1252";
    }

    inline QByteArray chooseCodec(std::pair<QLocale, QLocale> pair){
        const auto orig = langToName(pair.first);
        const auto local = langToName(pair.second);

        if(orig == local)
            return orig;
        else if(orig != "windows-1252" &&
                local != "windows-1252")
            return orig;
        else if(orig == "windows-1252" && !local.isEmpty())
            return local;
        else
            return orig;
    }
};
#endif // LANGUAGES_H
