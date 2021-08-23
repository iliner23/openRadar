#ifndef REPERTORYDATA_H
#define REPERTORYDATA_H
#include "commonfunctions.h"

namespace functions{
    class repertoryData
    {
    public:
        repertoryData() = default;
        repertoryData(const openCtree & repertory, QTextCodec * codec = QTextCodec::codecForName("system"));
        void reset(const openCtree & repertory, QTextCodec * codec = QTextCodec::codecForName("system"));
        void clear();
        bool isClear() const { return _data.isEmpty(); }

        quint16 labelIndexFilter() const;
        quint16 maxDrugs() const;
        quint16 childrenLabels() const;

        quint8 level() const;//attach
        quint8 chapterNumber() const;

        QByteArray rawData() const;

        QByteArray firstCurrentIndex() const;
        QByteArray secondCurrentIndex() const;
        QByteArray parentIndex() const;
        std::string key() const;

        std::pair<QString, QString> repertoryLabel() const;
        std::pair<QString, QString> synonymsList() const;
        std::pair<QStringList, QStringList> linksList() const;
        std::pair<QStringList, QStringList> crossLinksList() const;

        QVector<std::tuple<quint16, quint8, quint16, quint16/*, quint16*/>> remedsList() const;

        class repertoryDataException : public std::exception {};
    private:
        inline void genLabels(const quint16 _Fpos, quint16 &_Spos);
        inline void genLinks(quint16 & _Tpos);
        inline void linksHandler(const quint8 type, const QString synLinkText);
        inline void genRemeds(quint16 _Tpos);

        std::pair<QString, QString> _labels;
        std::pair<QString, QString> _synonyms;
        std::pair<QStringList, QStringList> _links;
        std::pair<QStringList, QStringList> _crossLinks;

        QVector<std::tuple<quint16, quint8, quint16, quint16/*, quint16*/>> _remeds;
        //remed, type, author, filter, remedPos

        std::string _key;
        QTextCodec * _codec;
        QByteArray _data;
    };
};

#endif // REPERTORYDATA_H
