#ifndef VOCABULARY_H
#define VOCABULARY_H

#include <QtWidgets>
#include <QTextCodec>
#include <QtConcurrent/QtConcurrent>
#include "openctree.h"
#include "languages.h"
#include "searchresult.h"

namespace Ui {
class vocabulary;
}

class vocabulary : public QDialog
{
    Q_OBJECT

public:
    explicit vocabulary(const QDir system, const QLocale::Language language,
                        const QDir catalog, QTextCodec *codec, QWidget *parent);
    ~vocabulary();
private:
    Ui::vocabulary *ui;
    QTextCodec * _codec = nullptr;
    QLocale::Language _lang;
    QDir _catalog, _system;

    QStringListModel * _model;
    QSortFilterProxyModel * _filter;

    searchResult * _results;

    bool _typeAdded = false;
    bool _globalHide = true;

    inline void renderingWords(openCtree);
    inline void renderingRoots(openCtree);
    inline void threadsLaunch(openCtree &, std::function<QStringList (openCtree, const int, const int)>);
    inline void clearList();
    inline void prepareOpen();
signals:
    void sendChoosenChapter(QByteArray);
private slots:
    void changedLanguage();
    void rendering(const int);
    void filter(const QString);
    void selectedModelItem(const QModelIndex &);
    void changedPlainText();
    void openResults();
public slots:
    void show();
    void open() override;
    void reject() override;
};

#endif // VOCABULARY_H
