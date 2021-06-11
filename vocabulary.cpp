#include "vocabulary.h"
#include "ui_vocabulary.h"

vocabulary::vocabulary(const QDir & system, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::vocabulary)
{
    ui->setupUi(this);
    setLayout(ui->verticalLayout);
    _system = system;

    ui->comboBox->insertItem(0, "Слова в репертории");
    ui->comboBox->insertItem(1, "Корни и формы");

    ui->comboBox_3->insertItem(0, "Оригинальный");
    ui->comboBox_3->insertItem(1, "Локализация");

    ui->comboBox_2->insertItems(0, {"и", "или"});

    _model = new QStringListModel(this);
    _filter = new QSortFilterProxyModel(this);
    _filter->setSourceModel(_model);
    _filter->setFilterCaseSensitivity(Qt::CaseInsensitive);

    ui->listView->setModel(_filter);
    ui->listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->listView->setUniformItemSizes(true);

    connect(ui->lineEdit, &QLineEdit::textChanged, this, &vocabulary::filter);
    connect(ui->listView->selectionModel(), &QItemSelectionModel::currentChanged, this, &vocabulary::selectedModelItem);
    connect(ui->plainTextEdit_2, &QPlainTextEdit::textChanged, this, &vocabulary::changedPlainText);
}
vocabulary::~vocabulary()
{
    delete ui;
}
void vocabulary::changedLanguage(){
    ui->plainTextEdit_2->clear();
    rendering(ui->comboBox->currentIndex());
}
void vocabulary::rendering(const int type){
    ui->listView->clearSelection();
    ui->lineEdit->clear();

    auto hidChange = [&](){
        if((ui->comboBox_3->isHidden()
                && ui->comboBox_3->currentIndex() != 0)){

            ui->comboBox_3->disconnect();
            ui->comboBox_3->setCurrentIndex(0);
            connect(ui->comboBox_3, &QComboBox::currentIndexChanged,
                    this, &vocabulary::changedLanguage);
        }
    };

    if(type == 0){
        auto word2 = openCtree(_catalog.filePath("word2").toStdString());
        bool hide = (word2.size() == 0) ? true : false;

        ui->label_3->setHidden(hide);
        ui->comboBox_3->setHidden(hide);
        hidChange();

        if(ui->comboBox_3->currentIndex() == 0)
            renderingWords(openCtree(_catalog.filePath("word1").toStdString()));
        else
            renderingWords(std::move(word2));
    }
    else if(type == 1){
        auto root2 = openCtree(_system.filePath("roots10").toStdString());
        bool hide = (root2.size() == 0) ? true : false;

        ui->label_3->setHidden(hide);
        ui->comboBox_3->setHidden(hide);
        hidChange();

        if(ui->comboBox_3->currentIndex() == 0)
            renderingRoots(openCtree(_system.filePath("roots1").toStdString()));
        else
            renderingRoots(std::move(root2));
    }
}
void vocabulary::renderingRoots(openCtree base){
    auto return_size = [](const auto & value, const auto & _size){
        return (value == -1) ? _size : value;
    };

    auto threadFunc = [&](openCtree tree, const int from, const int until){
        QStringList list;

        for(auto i = from; i != until; ++i){
            QByteArray str;

            if(i == from)
                str = QByteArray::fromStdString(tree.at(i));
            else
                str = QByteArray::fromStdString(tree.next());

            //root word
            auto lastTextPos = return_size(
                str.indexOf('\0', tree.serviceDataLenght()), str.size());

            if(lastTextPos == str.size())
                continue;

            auto localed = _codec->toUnicode(str.mid(tree.serviceDataLenght(),
                          lastTextPos - tree.serviceDataLenght()));

            list.push_back(localed);

            //synonym words
            while(lastTextPos < str.size()){
                const auto startPos = lastTextPos + 1;

                lastTextPos = return_size(
                            str.indexOf('|', startPos), str.size());

                localed = _codec->toUnicode(str.mid(startPos,
                            lastTextPos - startPos));

                list.push_back("  " + localed);
            }
        }

        return list;
    };

    threadsLaunch(base, threadFunc);
}
void vocabulary::renderingWords(openCtree base){
    auto return_size = [](const auto & value, const auto & _size){
        return (value == -1) ? _size : value;
    };

    auto threadFunc = [&](openCtree tree, const int from, const int until){
        QStringList list;

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

            list.push_back(localed);
        }

        return list;
    };

    threadsLaunch(base, threadFunc);
}
void vocabulary::threadsLaunch(openCtree & base, std::function<QStringList(openCtree, const int, const int)> threadFunc){
    QStringList wordList;

    if(base.size() > 20){
        const auto del = base.size() / 4;
        auto thread1 = QtConcurrent::run(threadFunc, base, 0 , del);
        auto thread2 = QtConcurrent::run(threadFunc, base, del , del * 2);
        auto thread3 = QtConcurrent::run(threadFunc, base, del * 2, del * 3);
        auto thread4 = QtConcurrent::run(threadFunc, base, del * 3, base.size());

        wordList.append(thread1.result());
        wordList.append(thread2.result());
        wordList.append(thread3.result());
        wordList.append(thread4.result());
    }
    else
        wordList = threadFunc(base, 0 , base.size());

    _model->setStringList(std::move(wordList));
}
void vocabulary::open(QMdiSubWindow* symptom){
    auto link = qobject_cast<repertory*>(symptom->widget());
    _catalog = link->catalog();
    _codec = link->textCodec();

    ui->plainTextEdit_2->clear();
    ui->comboBox->setCurrentIndex(0);
    ui->comboBox_3->setCurrentIndex(0);

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
    ui->comboBox_3->disconnect();
    ui->comboBox->disconnect();
    _model->setStringList(QStringList());

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

    if(ui->comboBox->currentIndex() == 0 || ui->comboBox->currentIndex() == 1){
        if(*text.constData() == ' ' && *(text.constData() + 1) == ' ')
            ui->plainTextEdit_2->insertPlainText("[word:" % text.mid(2) % "]");
        else
            ui->plainTextEdit_2->insertPlainText("[word:" % text % "]");
    }

    _typeAdded = true;
}
void vocabulary::changedPlainText(){
    if(ui->plainTextEdit_2->toPlainText().isEmpty())
        _typeAdded = false;
}
