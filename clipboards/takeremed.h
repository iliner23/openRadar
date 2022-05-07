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
    explicit takeRemed(std::shared_ptr<QStringList> name, QWidget *parent = nullptr);
    void setPos(QDir repertory, QByteArray key, QTextCodec *codec);
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

    void showEvent(QShowEvent *event) override;
    std::shared_ptr<QStringList> _name;
    QDir _repertory;
    QByteArray _key;
    QTextCodec * _codec;
};

#endif // TAKEREMED_H
