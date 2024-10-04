#ifndef PHOTOVIEWER_H
#define PHOTOVIEWER_H

#include <QWidget>
#include <QLabel>

namespace Ui {
class photoViewer;
}

class photoViewer : public QWidget
{
    Q_OBJECT

public:
    explicit photoViewer(QWidget *parent = nullptr);
    ~photoViewer();
public slots:
    void show(const QList<QPixmap> &);
private slots:
    void back();
    void forward();

private:
    Ui::photoViewer *ui;
    int _pos = 0;
    QList<QPixmap> _photos;

    void setPos(int);
};

#endif // PHOTOVIEWER_H
