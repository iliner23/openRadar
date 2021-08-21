#ifndef LANGUAGES_H
#define LANGUAGES_H
#include <QLocale>
#include <QTextCodec>
#include <array>

namespace languages {
    const std::array<QLocale::Language, 13> radarLang =
    {
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

    inline QByteArray languageToName(QLocale::Language lang){
       switch (lang) {
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
           return "";
       }
    }

    inline QByteArray systemCodec(){
        return "windows-1252";
    }

    inline QByteArray chooseCodec(std::pair<QLocale::Language, QLocale::Language> pair){
        const auto orig = languageToName(pair.first);
        const auto local = languageToName(pair.second);

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
