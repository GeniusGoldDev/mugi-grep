#include "widget/searchbrowser.h"

#include <QFont>
#include <QDebug>

SearchBrowser::SearchBrowser(RegExp exp, RegExpPath filter, int linesBefore, int linesAfter, QWidget *parent) :
    QTextBrowser(parent), mExp(exp), mFilter(filter), mLinesBefore(linesBefore), mLinesAfter(linesAfter)
{
    setOpenLinks(false);

    //@todo external stylesheet
    QFont font("Courier New",12,QFont::Normal);
    setFont(font);
}

SearchBrowser::~SearchBrowser()
{
    qDebug() << "~SearchBrowser()";
}

RegExp SearchBrowser::exp() const
{
    return mExp;
}

RegExpPath SearchBrowser::filter() const
{
    return mFilter;
}

int SearchBrowser::linesBefore() const
{
    return mLinesBefore;
}

int SearchBrowser::linesAfter() const
{
    return mLinesAfter;
}
