#include "keyreader.h"
#include "ui_keyreader.h"

keyReader::keyReader(QByteArray text, QString title, QDialog * keyRemedList, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::keyReader)
{
    ui->setupUi(this);
    _text = text;
    _codec = QTextCodec::codecForName(lang::defaultCodec());
    _keyRemedList = keyRemedList;
    setWindowTitle(title);

    ui->plainTextEdit->setPlainText(_codec->toUnicode(text));
}
void keyReader::setCodec(){
    using namespace lang;
    QStringList languages;

    for(auto & it : lang::radarLang){
        if(QTextCodec::codecForName(langToName(it)) != nullptr)
            languages += QLocale(it).nativeLanguageName();
    }

    bool ok;
    const auto codec =
            QInputDialog::getItem(this, "Выберите язык",
                    "Выберите язык отображения репертория", languages, 0, false, &ok);

    const auto pos = languages.indexOf(codec);

    if(ok){
        _codec = QTextCodec::codecForName(
                    langToName(pos));
        ui->plainTextEdit->setPlainText(_codec->toUnicode(_text));
    }
}
void keyReader::showKeyChoose(){
    QDialog::accept();
    _keyRemedList->exec();
}
keyReader::~keyReader(){
    delete ui;
}
