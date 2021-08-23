#include "remed_author.h"
#include "ui_remed_author.h"

remed_author::remed_author(const QDir path, std::shared_ptr<func::cache> ch,
                           const QByteArray pos,
                           const quint16 remFilter, const quint32 localPos, keysRemedList * remedList, QTextCodec *codec, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::remed_author)
{
    ui->setupUi(this);
    _path = path;
    _pos = pos;
    _remFilter = remFilter;
    _localPos = localPos;
    _cache = ch;
    _codec = codec;
    _remedList = remedList;

    _sym.open(path.filePath("symptom").toStdString());

    ui->listWidget_2->addItem("Авторы");
    ui->listWidget_2->addItem("Семейства");
    ui->label->setFont(QFont("defalut", 10));
    ui->label_2->setFont(QFont("defalut", 10));
    rendering();

    connect(ui->listWidget_2, &QListWidget::itemSelectionChanged, this, &remed_author::showTextInformation);
    connect(ui->pushButton, &QPushButton::clicked, this, &remed_author::showRemedList);

    ui->listWidget_2->setCurrentRow(0);
}

void remed_author::showTextInformation(){
    ui->listWidget->clear();
    switch (ui->listWidget_2->currentRow()){
    case 0:{
        for(const auto & it : qAsConst(_authorsText))
            ui->listWidget->addItem(it);
        break;
    }
    case 1:
        break;
    }
}
void remed_author::rendering(){
    //remed
    auto authorTxt = [&](const auto & text){
        QTextCodec * cd = QTextCodec::codecForName(lang::defaultCodec());
        QString tmp;
        auto itA = std::find(text.cbegin() + _cache->_lenAuthor, text.cend(), '\0');
        tmp += "(" + QString::fromStdString(std::string(text.cbegin() + _cache->_lenAuthor, itA)) + ")";

        if(itA != text.cend()){
            ++itA;
            auto itB = std::find(itA, text.cend(), '\0');
            tmp += " " + cd->toUnicode(std::string(itA, itB).c_str());
            itA = itB;
        }
        if(itA != text.cend()){
            ++itA;
            auto itB = std::find(itA, text.cend(), '\0');
            tmp += "\n" + cd->toUnicode(std::string(itA, itB).c_str());
            itA = itB;
        }

        _authorsText.push_back(std::move(tmp));
    };

    auto remedTxt = [&](const auto & text){
        QString tmp;
        auto itA = std::find(text.cbegin() + _cache->_lenRem, text.cend(), '\0');
        tmp += QString::fromStdString(std::string(text.cbegin() + _cache->_lenRem, itA));

        _remedName = tmp;
        _remedName.chop(1);

        if(itA != text.cend()){
            ++itA;
            auto itB = std::find(itA, text.cend(), '\0');
            tmp += " - " + QString::fromStdString(std::string(itA, itB).c_str());
        }

        return tmp;
    };

    _sym.at(_pos.toStdString(), false);
    functions::repertoryData symptom(_sym, _codec);

    quint16 prevRem = 0;
    const auto remeds = symptom.remedsList();

    for(auto it = _localPos; it != remeds.size(); ++it){
        auto rm = remeds.at(it);

        if(it != _localPos && prevRem != std::get<0>(rm))
            break;

        if(_remFilter == (quint16)-1 || (std::get<3>(rm) & _remFilter) != 0){
            if((((char *)&std::get<2>(rm))[1] & (char)128) != 0){
                ((char *)&std::get<2>(rm))[1] ^= (char)128;//remed have * in the end
            }

            const auto & rmStr = _cache->_cacheRemed.at(std::get<0>(rm));
            const auto & auStr = _cache->_cacheAuthor.at(std::get<2>(rm));

            if(prevRem != std::get<0>(rm))
                ui->label_2->setText(remedTxt(rmStr));

            authorTxt(auStr);
            prevRem = std::get<0>(rm);
        }
    }

    ui->label->setText(func::renderingLabel(_sym, false, _codec));
}
void remed_author::showRemedList(){
    _remedList->setFilterRemed(_remedName);
    _remedList->exec();
}
remed_author::~remed_author(){
    delete ui;
}
