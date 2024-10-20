#ifndef ENABLEDMEASURE_H
#define ENABLEDMEASURE_H

#include <QDialog>

namespace Ui {
class enabledMeasure;
}

class enabledMeasure : public QDialog
{
    Q_OBJECT

public:
    explicit enabledMeasure(QWidget *parent = nullptr);
    std::array<bool, 4> getValues() const;
    void setValues(const std::array<bool, 4> &);
    ~enabledMeasure();

private:
    Ui::enabledMeasure *ui;
};

#endif // ENABLEDMEASURE_H
