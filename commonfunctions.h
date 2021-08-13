#ifndef COMMONFUNCTIONS_H
#define COMMONFUNCTIONS_H
#include <QtCore>
#include <QTextCodec>
#include <QtConcurrent/QtConcurrent>
#include "openctree.h"

namespace functions {
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
}
#endif // COMMONFUNCTIONS_H
