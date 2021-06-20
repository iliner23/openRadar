#include "repchose.h"
#include "ui_repchose.h"

RepChose::RepChose(const QStringList repertories, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RepChose)
{
    ui->setupUi(this);

    openCtree view;
    uint8_t count = 0;
    QString temp;
    QStringList items;

    _codec = QTextCodec::codecForName("system");

    for(auto & it: repertories){
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
            view.open(QDir::toNativeSeparators(it + "/view").toStdString());

            if(view.size() != 0)
                _rLevels.push_back(QDir::toNativeSeparators(it + "/view"));
            else
                _rLevels.push_back("");

            view.close();
            count = 0;
        }
    }

    _model = new QStringListModel(this);
    _proxyModel = new QSortFilterProxyModel(this);
    _model->setStringList(items);
    _proxyModel->setSourceModel(_model);
    _proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    ui->listView->setModel(_proxyModel);
    ui->listView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(ui->lineEdit, &QLineEdit::textChanged, this, &RepChose::finding);
    connect(ui->listView->selectionModel(), &QItemSelectionModel::currentChanged, this, &RepChose::activateLevel);
    connect(ui->listView, &QListView::activated, this, &RepChose::accept);
    connect(ui->pushButton_3, &QPushButton::clicked, this, &RepChose::showLevels);
    connect(ui->pushButton_4, &QPushButton::clicked, this, &RepChose::setCodec);
}
void RepChose::accept(){
    QDialog::accept();
    auto list = _proxyModel->mapToSource(ui->listView->currentIndex());
    emit chooseRep(list, _repLevel, _codec);
    _repLevel = -1;
}
void RepChose::setCodec(){
    const auto codecsArray = QTextCodec::availableCodecs();
    QStringList codecList;
    const auto localCodec = codecsArray.indexOf(_codec->name());

    for(const auto & it : codecsArray)
        codecList.push_back(QString::fromLocal8Bit(it));

    const auto codec =
            QInputDialog::getItem(this, "Выберите кодировку",
                    "Выберите кодировку для открытия репертория", codecList, localCodec, false);

    _codec = QTextCodec::codecForName(codec.toLocal8Bit());
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
void RepChose::sendLevel(const quint16 repLevel){
    _repLevel = repLevel;
    accept();
}
