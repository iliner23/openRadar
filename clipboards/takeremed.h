#ifndef TAKEREMED_H
#define TAKEREMED_H

#include <QtWidgets>
#include "commonfunction/commonfunctions.h"

namespace Ui {
class takeRemed;
}

class takeRemed : public QDialog
{
    Q_OBJECT

public:
    explicit takeRemed(QStringList name, QWidget *parent = nullptr);
    void setPos(QDir repertory, QByteArray key, QTextCodec *codec);
    ~takeRemed();
private slots:
    void setDefault();
public slots:
    void setClipboardsName(QStringList name);
    void accept() override;
signals:
    void addedClipboardsRemed(func::remedClipboardInfo, quint8);
    void changeClipboardsName(QStringList);
protected:
    Ui::takeRemed *ui;

    void showEvent(QShowEvent *event) override;
    QStringList _name;
    QDir _repertory;
    QByteArray _key;
    QTextCodec * _codec;
};

#endif // TAKEREMED_H
