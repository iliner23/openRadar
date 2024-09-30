#include "repchose.h"
#include "ui_repchose.h"

RepChose::RepChose(const QStringList repertories, const QVector<QDir> reppos, QVector<std::pair<QLocale, QLocale>> lang, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RepChose)
{
    ui->setupUi(this);

    openCtree view;

    _lang = lang;
    _codec = QTextCodec::codecForName(lang::defaultCodec());

    for(auto & it : reppos){
        view.open(it.filePath("view").toStdString());

        if(view.size() != 0)
            _rLevels.push_back(it.filePath("view"));
        else
            _rLevels.push_back("");

        view.close();
    }

    _model = new QStringListModel(this);
    _proxyModel = new QSortFilterProxyModel(this);
    _model->setStringList(repertories);
    _proxyModel->setSourceModel(_model);
    _proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    ui->listView->setModel(_proxyModel);
    ui->listView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(ui->listView->selectionModel(), &QItemSelectionModel::currentChanged, this, &RepChose::activateLevel);
}
void RepChose::accept(){
    QDialog::accept();
    auto list = _proxyModel->mapToSource(ui->listView->currentIndex());
    emit chooseRep(list, _repLevel, _lang.at(list.row()));
    _repLevel = -1;
}
RepChose::~RepChose()
{
    delete ui;
}
void RepChose::finding(const QString str){
    _proxyModel->setFilterFixedString(str);

    if(ui->listView->currentIndex().isValid()){
        const auto index = _proxyModel->mapToSource(ui->listView->currentIndex());
        ui->pushButton_3->setEnabled(!_rLevels.at(index.row()).isEmpty());
    }
    else{
        ui->pushButton_3->setEnabled(false);
        ui->pushButton_2->setEnabled(false);
    }
}
void RepChose::activateLevel(const QModelIndex & selected){
    if(!selected.isValid())
        return;

    ui->pushButton_3->setEnabled(!_rLevels.at(_proxyModel->mapToSource(selected).row()).isEmpty());
    ui->pushButton_2->setEnabled(true);
}
void RepChose::showLevels(){
    const auto index = _proxyModel->mapToSource(ui->listView->currentIndex());
    QStringList levels;
    openCtree view(_rLevels.at(index.row()).toStdString());

    _codec = QTextCodec::codecForName(
                lang::chooseCodec(_lang.at(index.row())));

    for(auto i = 0; i != view.size(); ++i){
        auto str = view.next();
        auto it = std::find(str.cbegin() + view.serviceDataLenght(), str.cend(), '\0');
        str = std::string(str.cbegin() + view.serviceDataLenght(), it);
        levels.push_back(_codec->toUnicode(str.c_str()));
    }

    bool ok = false;
    const auto codec =
            QInputDialog::getItem(this, "Уровни репертория",
                    "Доступные уровни доверия реперторию", levels, 0, false, &ok);

    if(!ok) return;

    _repLevel = 1;
    const auto maxRow = levels.indexOf(codec);

    for(auto i = 0; i != maxRow; ++i)
        _repLevel <<= 1;

    accept();
}
