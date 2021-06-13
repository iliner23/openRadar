#ifndef SEARCHRESULT_H
#define SEARCHRESULT_H

#include <QtWidgets>
#include <QTextCodec>
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

    inline void expressionParser(const QString expr);
    QVector<QByteArray> keysParser(const std::string & key, QSet<QByteArray> &set);
    inline void logicalParser();
    inline void clearValues();
};

#endif // SEARCHRESULT_H
