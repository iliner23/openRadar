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
        linksParser() = delete;
        static std::pair<QStringList, QVector<QByteArray>> mainParser(
                openCtree symptom, openCtree word, const QString expression,
                QTextCodec * codec = QTextCodec::codecForName("system"));
    private:
        enum class operation : quint8 {AND, OR, none};

        static void expressionParser(const QString, QVector<std::pair<QString, operation>> &);
        static QVector<QByteArray> keysParser(openCtree &, const std::string &, QSet<QByteArray> &);

        static void logicalANDparser(openCtree &, const QVector<QByteArray>,
                                     const QVector<QByteArray>, QVector<QByteArray> &);

        static std::pair<QStringList, QVector<QByteArray>> logicalParser(openCtree &, openCtree &,
                                        QVector<std::pair<QString, operation>> &, QTextCodec * codec);
        static void clearValues();
        static QVector<QVector<QByteArray>> threadsParent(openCtree &, const QVector<QByteArray> & ,
                        std::function<QVector<QVector<QByteArray>>(openCtree symptom,
                        const QVector<QByteArray> & , quint32 , quint32)>);
    };
}
#endif // COMMONFUNCTIONS_H
