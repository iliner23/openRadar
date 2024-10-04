#include "keychoose.h"
#include "ui_keychoose.h"

KeyChoose::KeyChoose(const QStringList keys, keysRemedList * list, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KeyChoose)
{
    ui->setupUi(this);

    _codec = QTextCodec::codecForName(lang::defaultCodec());

    _remedList = list;
    _model = new QStringListModel(this);
    _proxyModel = new QSortFilterProxyModel(this);
    _model->setStringList(keys);
    _proxyModel->setSourceModel(_model);
    _proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    ui->listView->setModel(_proxyModel);
    ui->listView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(ui->listView->selectionModel(), &QItemSelectionModel::currentChanged, this, &KeyChoose::activateLevel);

    ui->buttonBox->button(ui->buttonBox->Ok)->setEnabled(false);
}
void KeyChoose::accept(){
    QDialog::accept();
    auto list = _proxyModel->mapToSource(ui->listView->currentIndex());
    _remedList->changeTable(list.row());
    _remedList->exec();
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
KeyChoose::~KeyChoose(){
    delete ui;
}
