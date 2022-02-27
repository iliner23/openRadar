#ifndef COMMONFUNCTIONS_H
#define COMMONFUNCTIONS_H
#include <QtCore>
#include <QTextCodec>
#include <QtConcurrent/QtConcurrent>
#include "openctree/openctree.h"

namespace func {
    QString renderingLabel(openCtree symptom, bool passLastChapter,
                           QTextCodec * codec = QTextCodec::codecForName("system"));

    std::pair<QStringList, QStringList> renderingLabel
        (openCtree symptom, QTextCodec * codec = QTextCodec::codecForName("system"));

    QByteArrayList getRootPath(openCtree symptom,
                                    quint16 deep = std::numeric_limits<quint16>::max());

    class linksParser{
    public:
        linksParser() = default;
        std::pair<QStringList, QByteArrayList> operator() (
                openCtree symptom, openCtree word, const QString expression,
                QTextCodec * codec = QTextCodec::codecForName("system"));
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
        QVector<QByteArrayList> threadsParent(const QByteArrayList & ,
                        std::function<QVector<QByteArrayList>(openCtree symptom,
                        const QByteArrayList & , quint32 , quint32)>);
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
        bool remGroup = false;
        quint16 remFilter = -1;
    };
}
#endif // COMMONFUNCTIONS_H
