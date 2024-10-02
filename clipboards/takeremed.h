#ifndef TAKEREMED_H
#define TAKEREMED_H

#include <QtWidgets>
#include <memory>
#include "commonfunction/commonfunctions.h"

namespace Ui {
class takeRemed;
}

class takeRemed : public QDialog
{
    Q_OBJECT
public:
    explicit takeRemed(std::shared_ptr<QStringList>, QWidget *parent = nullptr);
    void setPos(QDir, QByteArray, quint16, QTextCodec *);
    ~takeRemed();
private slots:
    void setDefault();
public slots:
    void setClipboardsName();
    void accept() override;
signals:
    void addedClipboardsRemed(func::remedClipboardInfo, quint8);
    void changeClipboardsName();
protected:
    Ui::takeRemed *ui;

    void showEvent(QShowEvent *) override;
    std::shared_ptr<QStringList> _name;
    QDir _repertory;
    QByteArray _key;
    QTextCodec * _codec;
    quint16 _filter = -1;
};

#endif // TAKEREMED_H
