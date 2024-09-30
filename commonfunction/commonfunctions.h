#ifndef COMMONFUNCTIONS_H
#define COMMONFUNCTIONS_H
#include <QtCore>
#include <QTextCodec>
#include "openctree/openctree.h"

#define _DEBUG_

namespace func {
#ifdef _DEBUG_
    const QString dataPath = "../";
#else
    const QString dataPath = "";
#endif

    QString renderingLabel(openCtree symptom, bool passLastChapter,
                           QTextCodec * codec = QTextCodec::codecForLocale());

    std::pair<QStringList, QStringList> renderingLabel
        (openCtree symptom, QTextCodec * codec = QTextCodec::codecForLocale());

    QByteArrayList getRootPath(openCtree symptom,
                                    quint16 deep = std::numeric_limits<quint16>::max());

    class linksParser{
    public:
        linksParser() = default;
        std::pair<QStringList, QByteArrayList> operator() (
                openCtree symptom, openCtree word, const QString expression,
                QTextCodec * codec = QTextCodec::codecForLocale());
    private:
        enum class operation : quint8 {AND, OR, none};
        openCtree _symptom, _word;
        QTextCodec * _codec;
        QMutex _syMut, _wordMut;

        void expressionParser(const QString, QVector<std::pair<QString, operation>> &);
        QByteArrayList keysParser(const std::string &, QSet<QByteArray> &);

        void logicalANDparser(const QByteArrayList,
                                     const QByteArrayList, QByteArrayList &);

        std::pair<QStringList, QByteArrayList> logicalParser(
                                        QVector<std::pair<QString, operation>> &);
        void clearValues();
    };

    struct cache{
        std::unordered_map<uint16_t, std::string> _cacheRemed;
        std::unordered_map<uint16_t, std::string> _cacheAuthor;
        uint16_t _lenRem, _lenAuthor;
    };

    struct remedClipboardInfo{
        QDir path;
        QByteArray key;
        QTextCodec * codec;
        quint8 intensity = 1;
        bool elim = false;
        bool cas = false;
        QString group;
        bool measure[4] = { true, true, true, true};
        quint16 remFilter = -1;

        friend QDataStream & operator<< (QDataStream & out, const remedClipboardInfo & obj) {
            QByteArray measure(reinterpret_cast<const char*>(obj.measure), sizeof(obj.measure));

            out << obj.path.path() << obj.key << reinterpret_cast<uintptr_t>(obj.codec)
                << obj.intensity << obj.elim
                << obj.cas << obj.group << measure << obj.remFilter;
            return out;
        }

        friend QDataStream & operator>> (QDataStream & in, remedClipboardInfo & obj) {
            QString path;
            QByteArray measure("\0", sizeof(obj.measure));
            uintptr_t codecPtr = 0;

            in >> path >> obj.key >> codecPtr
                >> obj.intensity >> obj.elim >> obj.cas
                >> obj.group >> measure >> obj.remFilter;

            obj.path.setPath(path);
            std::copy(measure.cbegin(), measure.cend(), obj.measure);
            obj.codec = reinterpret_cast<QTextCodec*>(codecPtr);
            return in;
        }
    };
}
#endif // COMMONFUNCTIONS_H
