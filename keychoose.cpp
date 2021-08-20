#include "keychoose.h"
#include "ui_keychoose.h"

KeyChoose::KeyChoose(const QStringList keys, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KeyChoose)
{
    ui->setupUi(this);

    _codec = QTextCodec::codecForName(
                languages::languageToName(languages::radarLang.at(0)));

    openCtree view;
    uint8_t count = 0;
    QString temp;
    QStringList items;
    QVector<QDir> keysPos;

    for(auto & it: keys){
        switch (count) {
        case 1:
            temp += " (" + it + ") ";
            ++count;
            items.push_back(temp);
            break;
        case 0:
            temp = it;
            ++count;
            break;
        case 2:
            keysPos.append(it);
            count = 0;
        }
    }

    _remedList = new keysRemedList(keysPos, items, parentWidget());
    _model = new QStringListModel(this);
    _proxyModel = new QSortFilterProxyModel(this);
    _model->setStringList(items);
    _proxyModel->setSourceModel(_model);
    _proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    ui->listView->setModel(_proxyModel);
    ui->listView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(ui->lineEdit, &QLineEdit::textChanged, this, &KeyChoose::finding);
    connect(ui->listView->selectionModel(), &QItemSelectionModel::currentChanged, this, &KeyChoose::activateLevel);
    connect(ui->listView, &QListView::activated, this, &KeyChoose::accept);

    ui->buttonBox->button(ui->buttonBox->Ok)->setEnabled(false);
}
void KeyChoose::accept(){
    auto list = _proxyModel->mapToSource(ui->listView->currentIndex());
    _remedList->changeTable(list.row());
    _remedList->show();
    QDialog::accept();
}
void KeyChoose::finding(const QString str){
    _proxyModel->setFilterFixedString(str);
    ui->buttonBox->button(ui->buttonBox->Ok)->
            setEnabled(ui->listView->currentIndex().isValid());
}
void KeyChoose::activateLevel(const QModelIndex & selected){
    if(!selected.isValid())
        return;

    ui->buttonBox->button(ui->buttonBox->Ok)->setEnabled(true);
}
KeyChoose::~KeyChoose()
{
    delete ui;
}
