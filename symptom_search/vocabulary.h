#ifndef VOCABULARY_H
#define VOCABULARY_H

#include <QtWidgets>
#include <QTextCodec>
#include <QtConcurrent/QtConcurrent>
#include "openctree/openctree.h"
#include "commonfunction/languages.h"
#include "searchresult.h"

namespace Ui {
class vocabulary;
}

class vocabulary : public QWidget
{
    Q_OBJECT

public:
    explicit vocabulary(const QDir system,
                        const std::pair<QLocale, QLocale> language,
                        const QDir catalog, QTextCodec *codec, QWidget *parent = nullptr);
    ~vocabulary();
private:
    Ui::vocabulary *ui;
    QTextCodec * _codec = nullptr;
    std::pair<QLocale, QLocale> _lang;
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
protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;
signals:
    void sendKey(QByteArray);
private slots:
    void changedLanguage();
    void rendering(const int);
    void filter(const QString);
    void selectedModelItem(const QModelIndex &);
    void changedPlainText();
    void openResults();
    void sendOn();
};

#endif // VOCABULARY_H
