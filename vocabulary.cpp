#include "vocabulary.h"
#include "ui_vocabulary.h"

vocabulary::vocabulary(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::vocabulary)
{
    ui->setupUi(this);
    setLayout(ui->verticalLayout);

    ui->comboBox->insertItem(0, "Слова в репертории");

    _word = new QStringListModel(this);
    _filter = new QSortFilterProxyModel(this);
    _filter->setSourceModel(_word);
    ui->listView->setModel(_filter);

    connect(ui->lineEdit, &QLineEdit::textChanged, this, &vocabulary::filter);
}
vocabulary::~vocabulary()
{
    delete ui;
}
void vocabulary::changedLanguage(const int language){
    _language = language;
    rendering(ui->comboBox->currentIndex());
}
void vocabulary::rendering(const int type){
    if(type == 0){
        renderingWords();
        _filter->setSourceModel(_word);
    }
}
void vocabulary::renderingWords(){
    auto return_size = [](const auto & value, const auto & _size){
        return (value == -1) ? _size : value;
    };

    QString lastValue;
    QStringList wordList;

    openCtree word(_catalogs.at(_pos).first.filePath(
                (!_language) ? "word1" : "word2").toStdString());

    for(auto i = 0; i != word.size(); ++i){
        const auto str = QByteArray::fromStdString(word.next());
        const auto lastTextPos = return_size(
            str.indexOf('\0', word.serviceDataLenght()), str.size());

        const auto localed = _catalogs.at(_pos).second
                ->toUnicode(str.mid(word.serviceDataLenght(),
                      lastTextPos - word.serviceDataLenght()));

        if(lastValue == localed)
            continue;

        lastValue = localed;
        wordList.push_back(std::move(localed));
    }

    _word->setStringList(std::move(wordList));
}
void vocabulary::open(QList<QMdiSubWindow*> windows, QMdiSubWindow* symptom){
    _catalogs.clear();
    const auto pos = windows.indexOf(symptom);

    for(const auto & it : windows){
        auto ptr = qobject_cast<repertory*>(it->widget());
        _catalogs.push_back(std::make_pair(ptr->catalog(), ptr->textCodec()));
    }

    _pos = pos;
    rendering(0);
    QDialog::open();
}
void vocabulary::filter(const QString & flt){
    _filter->setFilterFixedString(flt);
}
