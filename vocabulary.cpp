#include "vocabulary.h"
#include "ui_vocabulary.h"

vocabulary::vocabulary(const QDir system, const QLocale::Language language, const QDir catalog, QTextCodec * codec, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::vocabulary)
{
    ui->setupUi(this);
    setWindowFlag(Qt::Window, true);

    if(parent != nullptr)
        setWindowTitle(windowTitle() + " - [" + parent->windowTitle() + ']');

    _lang = language;

    _system = system;
    _catalog = catalog;
    _codec = codec;
    _results = new searchResult(this);

    auto word2 = openCtree(_catalog.filePath("word2").toStdString());
    _globalHide = (word2.size() == 0) ? true : false;

    ui->comboBox->insertItem(0, "Слова в репертории");
    ui->comboBox->insertItem(1, "Корни и формы");
    ui->comboBox->insertItem(2, "Синонимы");

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
    connect(ui->listView, &QListView::clicked, this, &vocabulary::selectedModelItem);
    connect(ui->plainTextEdit_2, &QPlainTextEdit::textChanged, this, &vocabulary::changedPlainText);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &vocabulary::openResults);
    connect(_results, &searchResult::accepted, this, &vocabulary::sendOn);
}
vocabulary::~vocabulary()
{
    delete _results;
    delete ui;
}
void vocabulary::sendOn(){
    emit sendKey(_results->key());
    ui->plainTextEdit_2->clear();
    hide();
}
void vocabulary::changedLanguage(){
    ui->plainTextEdit_2->clear();
    rendering(ui->comboBox->currentIndex());
}
void vocabulary::rendering(const int type){
    ui->listView->clearSelection();
    ui->lineEdit->clear();
////////////////////////////////////////////////////////////////////////////////////////////////////////////
    auto hidChange = [&](){
        if((ui->comboBox_3->isHidden()
                && ui->comboBox_3->currentIndex() != 0)){

            ui->comboBox_3->disconnect();
            ui->comboBox_3->setCurrentIndex(0);
            connect(ui->comboBox_3, qOverload<int>(&QComboBox::currentIndexChanged),
                    this, &vocabulary::changedLanguage);
        }
    };

    auto synomRoots = [&](const QString & name){
        using namespace languages;
        const auto langIter = std::find(radarLang.cbegin(),
                                radarLang.cend(), _lang) - radarLang.cbegin() + 1;

        openCtree root2;
        bool hide;
        const auto localFile = (_system.filePath(name % QString::number(langIter)));

        if(!_globalHide && QFileInfo::exists(localFile % ".dat") && QFileInfo::exists(localFile % ".idx")){
            root2 = openCtree(localFile.toStdString());
            hide = (root2.size() == 0) ? true : false;
        }
        else
            hide = true;

        ui->label_3->setHidden(hide);
        ui->comboBox_3->setHidden(hide);
        hidChange();

        if(ui->comboBox_3->currentIndex() == 0)
            renderingRoots(openCtree(_system.filePath(name % '1').toStdString()));
        else
            renderingRoots(std::move(root2));
    };
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(type == 0){
        ui->label_3->setHidden(_globalHide);
        ui->comboBox_3->setHidden(_globalHide);
        hidChange();

        if(ui->comboBox_3->currentIndex() == 0)
            renderingWords(openCtree(_catalog.filePath("word1").toStdString()));
        else
            renderingWords(openCtree(_catalog.filePath("word2").toStdString()));
    }
    else if(type == 1)
        synomRoots("roots");
    else if(type == 2)
        synomRoots("thes");
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
    QVector<QFuture<QStringList>> threads;

    const auto maxThreads = (base.size() > QThread::idealThreadCount()) ?
                QThread::idealThreadCount() : base.size();
    const auto del = base.size() / maxThreads;

    for(auto i = 0; i != maxThreads; ++i){
        threads.append(QtConcurrent::run(threadFunc, base, del * i ,
            (i == maxThreads - 1) ? base.size() : del * (i + 1)));
    }

    for(auto & it : threads)
        wordList.append(it.result());

    _model->setStringList(std::move(wordList));
}
void vocabulary::filter(const QString flt){
    ui->listView->selectionModel()->clearCurrentIndex();
    _filter->setFilterFixedString(flt);
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

    if(ui->comboBox->currentIndex() == 0 ||
       ui->comboBox->currentIndex() == 1 ||
       ui->comboBox->currentIndex() == 2){
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
void vocabulary::prepareOpen(){
    ui->comboBox->setCurrentIndex(0);
    ui->comboBox_3->setCurrentIndex(0);

    connect(ui->comboBox_3, qOverload<int>(&QComboBox::currentIndexChanged), this, &vocabulary::changedLanguage);
    connect(ui->comboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &vocabulary::rendering);

    rendering(0);
}
void vocabulary::clearList(){
    ui->comboBox_3->disconnect();
    ui->comboBox->disconnect();
    _model->setStringList(QStringList());
}
void vocabulary::closeEvent(QCloseEvent *event){
    clearList();
    event->accept();
}
void vocabulary::showEvent(QShowEvent *event){
    prepareOpen();
    event->accept();
}
void vocabulary::openResults(){
    QFileInfo word, symptom(_catalog.filePath("symptom"));

    if(ui->comboBox_3->currentIndex() == 0)
        word.setFile(_catalog.filePath("word1"));
    else
        word.setFile(_catalog.filePath("word2"));

    _results->setData(word, symptom, ui->plainTextEdit_2->toPlainText(), _codec);
    _results->open();
}
