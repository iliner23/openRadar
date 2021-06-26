#ifndef COMMONFUNCTIONS_H
#define COMMONFUNCTIONS_H
#include <QtCore>
#include <QTextCodec>
#include <QtConcurrent/QtConcurrent>
#include "openctree.h"

namespace functions {
    QString renderingLabel(openCtree & symptom, bool passLastChapter = true,
                           QTextCodec * codec = QTextCodec::codecForName("system"));

    QVector<QByteArray> getRootPath(openCtree & symptom,
                                    quint16 deep = std::numeric_limits<quint16>::max());

    class linksParser{
    public:
        linksParser() = default;
        std::pair<QStringList, QVector<QByteArray>> operator() (
                openCtree symptom, openCtree word, const QString expression,
                QTextCodec * codec = QTextCodec::codecForName("system"));
    private:
        enum class operation : quint8 {AND, OR, none};
        openCtree _symptom, _word;
        QTextCodec * _codec;
        QMutex _syMut, _wordMut;

        void expressionParser(const QString, QVector<std::pair<QString, operation>> &);
        QVector<QByteArray> keysParser(const std::string &, QSet<QByteArray> &);

        void logicalANDparser(const QVector<QByteArray>,
                                     const QVector<QByteArray>, QVector<QByteArray> &);

        std::pair<QStringList, QVector<QByteArray>> logicalParser(
                                        QVector<std::pair<QString, operation>> &);
        void clearValues();
        QVector<QVector<QByteArray>> threadsParent(const QVector<QByteArray> & ,
                        std::function<QVector<QVector<QByteArray>>(openCtree symptom,
                        const QVector<QByteArray> & , quint32 , quint32)>);
    };
}
#endif // COMMONFUNCTIONS_H
