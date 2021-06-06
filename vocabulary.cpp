#include "vocabulary.h"
#include "ui_vocabulary.h"

vocabulary::vocabulary(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::vocabulary)
{
    ui->setupUi(this);
    setLayout(ui->verticalLayout);

    ui->comboBox->insertItem(0, "Слова в репертории");

    ui->comboBox_3->insertItem(0, "Оригинальный");
    ui->comboBox_3->insertItem(1, "Локализация");

    ui->comboBox_2->insertItems(0, {"и", "или"});

    _model = new QStringListModel(this);
    _filter = new QSortFilterProxyModel(this);
    _filter->setSourceModel(_model);
    _filter->setFilterCaseSensitivity(Qt::CaseInsensitive);

    ui->listView->setModel(_filter);
    ui->listView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(ui->lineEdit, &QLineEdit::textChanged, this, &vocabulary::filter);
    connect(ui->listView->selectionModel(), &QItemSelectionModel::currentChanged, this, &vocabulary::selectedModelItem);
    connect(ui->pushButton, &QPushButton::clicked, this, &vocabulary::clearPlainText);
}
vocabulary::~vocabulary()
{
    delete ui;
}
void vocabulary::changedLanguage(){
    clearPlainText();
    _typeAdded = false;
    rendering(ui->comboBox->currentIndex());
}
void vocabulary::rendering(const int type){
    ui->listView->clearSelection();
    ui->lineEdit->clear();

    if(type == 0){
        if(ui->comboBox_3->currentIndex() == 0)
            renderingWords(openCtree(_catalog.filePath("word1").toStdString()));
        else
            renderingWords(openCtree(_catalog.filePath("word2").toStdString()));

        _filter->setSourceModel(_model);
    }
}
void vocabulary::renderingWords(openCtree && base){
    auto return_size = [](const auto & value, const auto & _size){
        return (value == -1) ? _size : value;
    };

    QStringList wordList(base.size());

    auto threadFunc = [&](openCtree tree, const int from, const int until){
        for(auto i = from; i != until; ++i){
            QByteArray str;

            if(i == from)
                str = QByteArray::fromStdString(tree.at(i));
            else
                str = QByteArray::fromStdString(tree.next());

            const auto lastTextPos = return_size(
                str.indexOf('\0', tree.serviceDataLenght()), str.size());

            const auto localed = _codec
                    ->toUnicode(str.mid(tree.serviceDataLenght(),
                          lastTextPos - tree.serviceDataLenght()));

            if(*(str.constData() + 6) != '\0')
                continue;

            wordList[i] = std::move(localed);
        }
    };

    if(base.size() > 20){
        const auto del = base.size() / 4;
        auto thread1 = QtConcurrent::run(threadFunc, base, 0 , del);
        auto thread2 = QtConcurrent::run(threadFunc, base, del , del * 2);
        auto thread3 = QtConcurrent::run(threadFunc, base, del * 2, del * 3);
        auto thread4 = QtConcurrent::run(threadFunc, base, del * 3, base.size());

        thread1.waitForFinished();
        thread2.waitForFinished();
        thread3.waitForFinished();
        thread4.waitForFinished();
    }
    else
        threadFunc(base, 0 , base.size());

    wordList.removeAll("");
    _model->setStringList(std::move(wordList));
}
void vocabulary::open(QMdiSubWindow* symptom){
    auto link = qobject_cast<repertory*>(symptom->widget());
    _catalog = link->catalog();
    _codec = link->textCodec();

    clearPlainText();
    _typeAdded = false;

    bool hide = (openCtree(
                     _catalog.filePath("word2").toStdString()).size() == 0)
                        ? true : false;

    ui->comboBox->setCurrentIndex(0);
    ui->comboBox_3->setCurrentIndex(0);
    ui->label_3->setHidden(hide);
    ui->comboBox_3->setHidden(hide);

    connect(ui->comboBox_3, &QComboBox::currentIndexChanged,
            this, &vocabulary::changedLanguage);
    connect(ui->comboBox, &QComboBox::currentIndexChanged,
            this, &vocabulary::rendering);

    rendering(0);
    QDialog::open();
}
void vocabulary::filter(const QString & flt){
    ui->listView->selectionModel()->clearCurrentIndex();
    _filter->setFilterFixedString(flt);
}
void vocabulary::reject(){
    disconnect(ui->comboBox_3, &QComboBox::currentIndexChanged,
            this, &vocabulary::changedLanguage);
    disconnect(ui->comboBox, &QComboBox::currentIndexChanged,
            this, &vocabulary::rendering);

    QDialog::reject();
}
void vocabulary::selectedModelItem(const QModelIndex & index){
    if(!index.isValid())
        return;

    const auto text = _filter->data(index).toString();

    if(_typeAdded){
        if(ui->comboBox_2->currentIndex() == 0)
            ui->plainTextEdit_2->insertPlainText(" AND ");
        else
            ui->plainTextEdit_2->insertPlainText(" OR ");
    }

    if(ui->comboBox->currentIndex() == 0)
        ui->plainTextEdit_2->insertPlainText("[word:" + text + "]");

    _typeAdded = true;
}
void vocabulary::clearPlainText(){
    ui->plainTextEdit_2->clear();
    _typeAdded = false;
}
