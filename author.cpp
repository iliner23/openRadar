#include "author.h"
#include "ui_author.h"

author::author(const quint32 authorPos, std::shared_ptr<cache> & ch, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::author)
{
    auto * locale = QTextCodec::codecForName("system");
    ui->setupUi(this);
    _cache = ch;
    _authorPos = authorPos;

    const auto & text = _cache->_cacheAuthor.at(authorPos);
    auto iter = text.cbegin() + _cache->_lenAuthor;
    quint16 date;
    ((quint8*) &date)[0] = *(iter - 3);
    ((quint8*) &date)[1] = *(iter - 2);

    QLabel * const lab[] = { ui->label_3, ui->label_4, ui->label_6, ui->label_5 };

    for(auto i = 0; i != 4; ++i){
        auto oldIter = iter;
        iter = std::find(oldIter, text.cend(), '\0');
        const auto tmpStr = locale->toUnicode(std::string(oldIter, iter).c_str());

        switch (i) {
        case 0 :
            lab[i]->setText("Ссылка : " + tmpStr);
            break;
        case 3 :
            if(date != 0){
                lab[i]->setText(QString::number(date) + " " + tmpStr);
                break;
            }
        default:
            lab[i]->setText(tmpStr);
        }

        if(iter != text.cend())
            ++iter;
    }
}

author::~author()
{
    delete ui;
}
