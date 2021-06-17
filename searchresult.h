#ifndef SEARCHRESULT_H
#define SEARCHRESULT_H

#include <QtWidgets>
#include <QTextCodec>
#include <QtConcurrent/QtConcurrent>
#include <openctree.h>
#include <repertoryengine.h>

namespace Ui {
class searchResult;
}

class searchResult : public QDialog
{
    Q_OBJECT

public:
    explicit searchResult(QWidget *parent = nullptr);
    explicit searchResult(const QFileInfo word, const QFileInfo symptom,
                          const QString expression,
                          QTextCodec * codec = QTextCodec::codecForName("system"),
                          QWidget *parent = nullptr);
    ~searchResult();
public slots:
    void setData(const QFileInfo word, const QFileInfo symptom,
                 const QString expression,
                 QTextCodec * codec = QTextCodec::codecForName("system"));
    void reject();
    void accept();
signals:
    void sendKey(QByteArray);
private:
    enum class operation : quint8 {AND, OR, none};

    Ui::searchResult *ui;
    openCtree _word, _symptom;
    QVector<std::pair<QString, operation>> _expr;
    QTextCodec * _codec;

    inline void expressionParser(const QString);
    inline QVector<QByteArray> keysParser(const std::string &, QSet<QByteArray> &);
    inline void logicalANDparser(const QVector<QByteArray>,
                                 const QVector<QByteArray>, QVector<QByteArray> &);
    inline void logicalParser();
    inline void clearValues();
    inline QVector<QVector<QByteArray>> threadsParent(const QVector<QByteArray> & ,
                    std::function<QVector<QVector<QByteArray>>(openCtree symptom,
                    const QVector<QByteArray> & , quint32 , quint32)>);
};

#endif // SEARCHRESULT_H
