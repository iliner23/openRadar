#include "repchose.h"
#include "ui_repchose.h"

RepChose::RepChose(const QStringList & repertories, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RepChose)
{
    ui->setupUi(this);
    ui->pushButton_2->setEnabled(false);
    ui->pushButton_3->setEnabled(false);
    _levels = new Levels(this);
    openCtree view;
    uint8_t count = 0;
    QString temp;
    QListWidgetItem * itm = nullptr;

    _codec = QTextCodec::codecForName("system");

    for(auto & it: repertories){
        switch (count) {
        case 1:
            temp += " (" + it + ") ";
            ++count;
            ui->listWidget->addItem(temp);
            break;
        case 0:
            itm = nullptr;
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
    connect(ui->lineEdit, &QLineEdit::textChanged, this, &RepChose::finding);
    connect(ui->listWidget, &QListWidget::currentItemChanged, this, &RepChose::activateLevel);
    connect(ui->pushButton_3, &QPushButton::clicked, this, &RepChose::showLevels);
    connect(_levels, &Levels::sendLevel, this, &RepChose::sendLevel);
    connect(ui->pushButton_4, &QPushButton::clicked, this, &RepChose::setCodec);
}
void RepChose::accept(){
    QDialog::accept();
    auto list = ui->listWidget->currentItem();
    emit chooseRep(list, _repLevel, _codec);
    _repLevel = -1;
}
void RepChose::setCodec(){
    const auto codecsArray = QTextCodec::availableCodecs();
    QStringList codecList;
    const auto localCodec = codecsArray.indexOf(_codec->name());

    for(const auto & it : codecsArray)
        codecList.push_back(QString::fromLocal8Bit(it));

    const auto codec = QInputDialog::getItem(this, "Выберите кодировку", "Выберите кодировку для открытия репертория", codecList, localCodec, false);
    _codec = QTextCodec::codecForName(codec.toLocal8Bit());
}
RepChose::~RepChose()
{
    delete ui;
}
void RepChose::finding(const QString & str){
    if(str.isEmpty()){
        for(auto it = 0; it != ui->listWidget->count(); ++it)
            ui->listWidget->item(it)->setHidden(false);
    }
    else{
        auto arr = ui->listWidget->findItems(str, Qt::MatchFlag::MatchContains);
        for(auto it = 0; it != ui->listWidget->count(); ++it){
            if(arr.indexOf(ui->listWidget->item(it)) == -1)
                ui->listWidget->item(it)->setHidden(true);
            else
                ui->listWidget->item(it)->setHidden(false);
        }
    }

    if(ui->listWidget->currentItem() != nullptr){
        ui->pushButton_3->setEnabled(!_rLevels.at(ui->listWidget->currentRow()).isEmpty() && !ui->listWidget->currentItem()->isHidden());
        ui->pushButton_2->setEnabled(!ui->listWidget->currentItem()->isHidden());
    }
    else{
        ui->pushButton_3->setEnabled(false);
        ui->pushButton_2->setEnabled(false);
    }
}
void RepChose::activateLevel(QListWidgetItem * line){
    ui->pushButton_3->setEnabled(!_rLevels.at(ui->listWidget->row(line)).isEmpty());
    ui->pushButton_2->setEnabled(true);
}
void RepChose::showLevels(){
    const auto row = ui->listWidget->currentRow();
    _levels->showLevels(_rLevels.at(row));
}
void RepChose::sendLevel(const quint16 repLevel){
    _repLevel = repLevel;
    accept();
}
