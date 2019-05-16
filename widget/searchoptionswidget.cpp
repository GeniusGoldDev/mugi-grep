#include "searchoptionswidget.h"
#include "ui_searchoptionswidget.h"

#include "searchbrowser.h"
#include "rxcollector.h"
#include <QDebug>
#include <QFileDialog>
#include <QDir>
#include "worker.h"
#include "anchorclickhandler.h"
#include "selectfilesdialog.h"


SearchOptionsWidget::SearchOptionsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SearchOptionsWidget)
{
    ui->setupUi(this);

}

SearchOptionsWidget::~SearchOptionsWidget()
{
    delete ui;
}

void SearchOptionsWidget::setBrowser(SearchBrowser *browser, bool setValues)
{
    mBrowser = browser;
    if (setValues) {
        //updateCompletions();
        mActive = false;
        ui->filter->setValue(browser->filter());
        ui->exp->setValue(browser->exp());
        ui->linesBefore->setValue(browser->linesBefore());
        ui->linesAfter->setValue(browser->linesAfter());
        ui->cacheFileList->setChecked(browser->cacheFileList());
        ui->notBinary->setChecked(browser->notBinary());
        ui->replacement->setText(browser->replacement());

        ui->filter->hide();
        ui->filter->show(); // force layout to recalculate
    }

    mActive = true;
}

void SearchOptionsWidget::init(Worker *worker, AnchorClickHandler* clickHandler) {

    connect(ui->filter,SIGNAL(returnPressed()),this,SIGNAL(search()));
    connect(ui->exp,SIGNAL(returnPressed()),this,SIGNAL(search()));
    connect(ui->filter,SIGNAL(textChanged()),this,SLOT(onFilterTextChanged()));
    connect(ui->exp,SIGNAL(textChanged()),this,SLOT(onExpTextChanged()));
    connect(ui->linesAfter,SIGNAL(valueChanged(int)),this,SLOT(onLinesAfterValueChanged()));
    connect(ui->linesBefore,SIGNAL(valueChanged(int)),this,SLOT(onLinesBeforeValueChanged()));
    connect(ui->cacheFileList,SIGNAL(clicked(bool)),this,SLOT(onCacheFileListClicked(bool)));
    connect(ui->notBinary,SIGNAL(clicked(bool)),this,SLOT(onNotBinaryClicked(bool)));
    connect(ui->filter,SIGNAL(caseClicked(bool)),this,SLOT(onFilterTextChanged()));
    connect(ui->exp,SIGNAL(caseClicked(bool)),this,SLOT(onExpTextChanged()));
    connect(ui->linesBefore,SIGNAL(returnPressed()),this,SIGNAL(search()));
    connect(ui->linesAfter,SIGNAL(returnPressed()),this,SIGNAL(search()));
    connect(ui->replacement,SIGNAL(returnPressed()),this,SIGNAL(preview()));
    connect(ui->replacement,SIGNAL(textChanged(QString)),this,SLOT(onReplacementTextChanged(QString)));

    mWorker = worker;
    mClickHandler = clickHandler;
    connect(this,SIGNAL(countMatchedFiles(QString,RegExpPath,bool)),
            mWorker,SLOT(onCountMatchedFiles(QString,RegExpPath,bool)));
    connect(mWorker,SIGNAL(count(int,int)),this,SLOT(onCountMatchedFiles(int,int)));

    ui->fileCount->setText(QString());
}


QString SearchOptionsWidget::path() const
{
    return ui->path->text();
}

void SearchOptionsWidget::setPath(const QString &path)
{
    ui->path->setText(path);
}

void SearchOptionsWidget::setBrowserValues()
{
    mBrowser->setFilter(ui->filter->value());
    mBrowser->setExp(ui->exp->value());
    mBrowser->setLinesBefore(ui->linesBefore->value());
    mBrowser->setLinesAfter(ui->linesAfter->value());
    mBrowser->setCacheFileList(ui->cacheFileList->isChecked());
    mBrowser->setNotBinary(ui->notBinary->isChecked());
    mBrowser->setReplacement(ui->replacement->text());
}

void SearchOptionsWidget::collect()
{
    RXCollector* collector = RXCollector::instance();
    //mActive = false;
    collector->collect(ui->exp->value());
    collector->collect(ui->filter->value());
    //updateCollector();
}

void SearchOptionsWidget::updateCompletions() {
    mActive = false;
    RXCollector* collector = RXCollector::instance();
    collector->load(ui->exp);
    collector->load(ui->filter);
    mActive = true;
}

void SearchOptionsWidget::setActive(bool active)
{
    mActive = active;
}

void SearchOptionsWidget::on_selectPath_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, QString(), ui->path->text());
    if (path.isEmpty()) {
        return;
    }
    ui->path->setText(QDir::toNativeSeparators(path));
}

void SearchOptionsWidget::on_search_clicked()
{
    emit search();
}

void SearchOptionsWidget::countMatchedFiles() {
    // todo cache
    if (!ui->cacheFileList->isChecked()) {
        ui->fileCount->setText(QString());
        return;
    }
    ui->fileCount->setText("? / ?");
    QString path = ui->path->text();
    RegExpPath filter = ui->filter->value();
    bool notBinary = ui->notBinary->isChecked();
    emit countMatchedFiles(path,filter,notBinary);
}

void SearchOptionsWidget::onFilterTextChanged() {
    if (!mActive || !mBrowser) {
        return;
    }
    if (mBrowser->isExecuted()) {
        emit clone();
        return;
    }
    countMatchedFiles();
    mBrowser->setFilter(ui->filter->value());
}

void SearchOptionsWidget::onExpTextChanged() {
    if (!mActive || !mBrowser) {
        return;
    }
    if (mBrowser->isExecuted()) {
        emit clone();
        return;
    }
    mBrowser->setExp(ui->exp->value());
    emitTabTitle();
}

void SearchOptionsWidget::emitTabTitle() {
    emit tabTitle(ui->exp->value().include(), mBrowser->isExecuted());
}

void SearchOptionsWidget::onLinesAfterValueChanged() {
    if (!mActive || !mBrowser) {
        return;
    }
    if (mBrowser->isExecuted()) {
        emit clone();
        return;
    }
    mBrowser->setLinesAfter(ui->linesAfter->value());
}

void SearchOptionsWidget::onLinesBeforeValueChanged() {
    if (!mActive || !mBrowser) {
        return;
    }
    if (mBrowser->isExecuted()) {
        emit clone();
        return;
    }
    mBrowser->setLinesBefore(ui->linesBefore->value());
}

void SearchOptionsWidget::onCacheFileListClicked(bool value) {
    if (!mActive || !mBrowser) {
        return;
    }
    if (mBrowser->isExecuted()) {
        emit clone();
        return;
    }
    mBrowser->setCacheFileList(value);
}



void SearchOptionsWidget::on_selectFiles_clicked()
{
    QString path = ui->path->text();
    SelectFilesDialog dialog(path, ui->filter->value(), mWorker, mClickHandler, this);
    if (dialog.exec() == QDialog::Accepted) {
        ui->filter->setValue(dialog.filter());
    }
}

void SearchOptionsWidget::onCountMatchedFiles(int matched, int total)
{
    ui->fileCount->setText(QString("%1 / %2 files").arg(matched).arg(total));
}

void SearchOptionsWidget::on_path_textChanged(const QString &path)
{
    emit pathChanged(path);
}

void SearchOptionsWidget::onNotBinaryClicked(bool value) {
    if (!mActive || !mBrowser) {
        return;
    }
    if (mBrowser->isExecuted()) {
        emit clone();
        return;
    }
    mBrowser->setNotBinary(value);
}

void SearchOptionsWidget::on_replace_clicked()
{
    emit replace();
}

void SearchOptionsWidget::on_preview_clicked()
{
    emit preview();
}

void SearchOptionsWidget::onReplacementTextChanged(QString value)
{
    if (!mActive || !mBrowser) {
        return;
    }
    if (mBrowser->isExecuted()) {
        emit clone();
        return;
    }
    mBrowser->setReplacement(value);
}
