#ifndef LANGUAGES_H
#define LANGUAGES_H
#include <QLocale>
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
};
#endif // LANGUAGES_H
