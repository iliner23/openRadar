#ifndef RESEARCHREMED_H
#define RESEARCHREMED_H

#include <QtWidgets>
#include "commonfunctions.h"

namespace Ui {
class researchRemed;
}

class researchRemed : public QMainWindow
{
    Q_OBJECT
    using rci = func::remedClipboardInfo;
public:
    explicit researchRemed(QStringList clipNames, QWidget *parent = nullptr);
    ~researchRemed();
public slots:
    void setClipboard(quint8 pos);
    void setClipboardName(QStringList);
    void setClipboardRemed(std::array<QVector<rci>, 10>);
private:
    Ui::researchRemed *ui;

    struct clipList{
        QWidget * widget;
        QLabel * label;
        QListWidget * list;
    };

    std::array<clipList, 10> _labels;

    std::array<QVector<rci>, 10> _clipboadrs;
    QStringList _clipNames;
    quint8 _pos = 0;
};

#endif // RESEARCHREMED_H
