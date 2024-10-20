#include "keysremedlist.h"
#include "ui_keysremedlist.h"

keysRemedList::keysRemedList(const QVector<QDir> keysFile, QStringList keysText, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::keysRemedList)
{
    ui->setupUi(this);

    _keysFile = keysFile;
    _keysText = keysText;
    _codec = QTextCodec::codecForName(lang::defaultCodec());

    _model = new QStandardItemModel(this);
    _proxy = new QSortFilterProxyModel(this);

    _proxy->setSourceModel(_model);
    ui->tableView->setModel(_proxy);
    ui->comboBox->insertItems(0, keysText);
    ui->buttonBox->button(ui->buttonBox->Ok)->setEnabled(false);

    connect(ui->comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &keysRemedList::renderingTable);
    connect(ui->tableView->selectionModel(), &QItemSelectionModel::currentChanged, this, &keysRemedList::activateOkBut);
}
void keysRemedList::changeTable(int pos){
    ui->comboBox->setCurrentIndex(pos);
}
void keysRemedList::renderingTable(int pos){
    _model->clear();

    if(pos <= -1)
        return;

    auto return_size = [](const auto & value, const auto & _size){
        return (value == -1) ? _size : value;
    };

    openCtree data(_keysFile.at(pos).path().toStdString());

    _model->setColumnCount(2);
    _model->setRowCount(data.size());

    auto threadFunc = [&](openCtree data, const auto begin, const auto end){
        QVector<std::pair<QStandardItem*, QStandardItem*>> items;

        for(auto i = begin; i != end; ++i){
            QByteArray str;

            if(i == begin)
                str = QByteArray::fromStdString(data.at(i));
            else
                str = QByteArray::fromStdString(data.next());

            str = str.mid(data.serviceDataLenght());
            const auto iter = return_size(str.indexOf('\0'), str.size());
            const auto secIter = return_size(str.indexOf('\0', iter + 1), str.size());

            const auto low = _codec->toUnicode(str.left(iter - 1));
            const auto full = _codec->toUnicode(str.mid(iter + 1, secIter - iter - 1));

            items += std::make_pair(new QStandardItem(low), new QStandardItem(full));
        }

        return items;
    };

    if(data.size() != 0){
        QVector<QFuture<
                QVector<std::pair<QStandardItem*,
                QStandardItem*>>>> threads;

        const auto maxThreads = (data.size() > QThread::idealThreadCount()) ?
                    QThread::idealThreadCount() : data.size();
        const auto del = data.size() / maxThreads;

        for(auto i = 0; i != maxThreads; ++i){
            threads.append(QtConcurrent::run(threadFunc, data, del * i,
                (i == maxThreads - 1) ? data.size() : del * (i + 1)));
        }

        int count = 0;

        for(auto & it : threads){
            auto vec = it.result();

            for(auto & iter : vec){
                _model->setItem(count, 0, iter.first);
                _model->setItem(count, 1, iter.second);
                ++count;
            }
        }
    }

    ui->buttonBox->button(ui->buttonBox->Ok)->setEnabled(false);
}
void keysRemedList::shortFilter(QString filter){
    disconnect(ui->lineEdit_2, &QLineEdit::textChanged, this, &keysRemedList::fullFilter);
    ui->lineEdit_2->clear();
    connect(ui->lineEdit_2, &QLineEdit::textChanged, this, &keysRemedList::fullFilter);

    _proxy->setFilterKeyColumn(0);
    _proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    _proxy->setFilterFixedString(filter);
}
void keysRemedList::fullFilter(QString filter){
    disconnect(ui->lineEdit, &QLineEdit::textChanged, this, &keysRemedList::shortFilter);
    ui->lineEdit->clear();
    connect(ui->lineEdit, &QLineEdit::textChanged, this, &keysRemedList::shortFilter);

    _proxy->setFilterKeyColumn(1);
    _proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    _proxy->setFilterFixedString(filter);
}
void keysRemedList::tableActivated(const QModelIndex & index){
    openReader(index);
}
void keysRemedList::accept(){
    const auto index = ui->tableView->currentIndex();
    openReader(index);
}
void keysRemedList::reject(){
    _model->clear();
    ui->lineEdit->clear();
    ui->lineEdit->clear();
    QDialog::reject();
}
void keysRemedList::openReader(const QModelIndex & index){
    if(!index.isValid())
        return;

    openCtree data(_keysFile.at(
                ui->comboBox->currentIndex())
                   .path().toStdString());

    const auto nativeRow = _proxy->mapToSource(index).row();
    auto str = QByteArray::fromStdString(data.at(nativeRow));
    str.chop(1);
    const auto pos = str.lastIndexOf('\0', str.size() - 2);

    const QString label = ui->comboBox->currentText() % ": " %
            _model->item(nativeRow, 1)->text();

    auto reader = new keyReader(str.mid(pos + 1), label,
                                this, parentWidget());
    reader->setAttribute(Qt::WA_DeleteOnClose);
    reader->show();

    _model->clear();
    ui->lineEdit->clear();
    ui->lineEdit->clear();

    QDialog::accept();
}
void keysRemedList::setFilterRemed(const QString name){
    ui->lineEdit->setText(name);
}
void keysRemedList::activateOkBut(const QModelIndex & index){
    ui->buttonBox->button(ui->buttonBox->Ok)->setEnabled(index.isValid());
}
void keysRemedList::showEvent(QShowEvent *event){
    if(_model->rowCount() == 0)
        renderingTable(ui->comboBox->currentIndex());

    event->ignore();
}
keysRemedList::~keysRemedList(){
    delete ui;
}
