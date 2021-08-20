#ifndef KEYREADER_H
#define KEYREADER_H

#include <QtWidgets>
#include <QTextCodec>
#include "languages.h"

namespace Ui {
class keyReader;
}

class keyReader : public QDialog
{
    Q_OBJECT

public:
    explicit keyReader(QByteArray text, QWidget *parent = nullptr);
    ~keyReader();
private slots:
    void setCodec();
private:
    Ui::keyReader *ui;
    QByteArray _text;
    QTextCodec * _codec;
};

#endif // KEYREADER_H
