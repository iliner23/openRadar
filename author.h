#ifndef AUTHOR_H
#define AUTHOR_H

#include <QDialog>
#include <memory>
#include <QTextCodec>
#include "commonfunctions.h"
#include "languages.h"

namespace Ui {
class author;
}

class author : public QDialog
{
    Q_OBJECT
public:
    explicit author(const quint32, std::shared_ptr<func::cache>, QWidget *parent = nullptr);
    ~author();

private:
    Ui::author *ui;
    std::shared_ptr<func::cache> _cache;
    quint32 _authorPos;
};

#endif // AUTHOR_H
