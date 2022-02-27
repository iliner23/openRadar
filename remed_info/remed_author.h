#ifndef REMED_AUTHOR_H
#define REMED_AUTHOR_H

#include <QtWidgets>
#include <QTextCodec>
#include <memory>
#include "commonfunction/commonfunctions.h"
#include "keys/keysremedlist.h"
#include "commonfunction/repertorydata.h"

namespace Ui {
class remed_author;
}

class remed_author : public QDialog
{
    Q_OBJECT
private:
    QDir _path;
    openCtree _sym;
    QByteArray _pos;
    QStringList _authorsText;
    QString _remedName;

    int _localPos;
    quint16 _remFilter = -1;

    std::shared_ptr<func::cache> _cache;
    QTextCodec * _codec;

    keysRemedList * _remedList;

    void rendering();
private slots:
    void showTextInformation();
    void showRemedList();
public:
    explicit remed_author(const QDir, std::shared_ptr<func::cache>,
                          const QByteArray, const quint16, const quint32, keysRemedList * remedList,
                          QTextCodec * codec,
                          QWidget *parent = nullptr);
    ~remed_author();

private:
    Ui::remed_author *ui;
};

#endif // REMED_AUTHOR_H
