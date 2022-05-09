#ifndef SETREMEDY_H
#define SETREMEDY_H

#include <QtWidgets>
#include "commonfunction/commonfunctions.h"

namespace Ui {
class setRemedy;
}

class setRemedy : public QDialog
{
    Q_OBJECT
    using rci = func::remedClipboardInfo;
public:
    explicit setRemedy(std::shared_ptr<std::array<QVector<rci>, 10>> clipRemed,
                       std::pair<qsizetype, qsizetype> pos, QWidget *parent = nullptr);
    ~setRemedy();
public slots:
    void accept() override;
signals:
    void clipboardRemedChanged();
private:
    Ui::setRemedy *ui;
    std::shared_ptr<std::array<QVector<rci>, 10>> _clipRemed;
    std::pair<qsizetype, qsizetype> _pos;
private slots:
    void setDefault();
};

#endif // SETREMEDY_H
