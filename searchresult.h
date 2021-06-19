#ifndef SEARCHRESULT_H
#define SEARCHRESULT_H

#include <QtWidgets>
#include <QTextCodec>
#include <QtConcurrent/QtConcurrent>
#include "commonfunctions.h"
#include "openctree.h"

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
    void reject() override;
    void accept() override;
private slots:
    void acceptKey(QListWidgetItem *);
signals:
    void sendKey(QByteArray);
private:
    Ui::searchResult *ui;
    openCtree _word, _symptom;
    QVector<QByteArray> _keys;
    QTextCodec * _codec;

    inline void clearValues();
};

#endif // SEARCHRESULT_H
