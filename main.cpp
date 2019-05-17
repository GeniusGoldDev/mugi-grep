#include <QApplication>
#include "widget/mainwindow.h"

#include <QTextCodec>

#include "regexp.h"
#include "regexppath.h"
#include "version.h"

#include "searchcache.h"
#include "searchparams.h"

int main(int argc, char *argv[])
{
    //QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    qRegisterMetaType<RegExp>();
    qRegisterMetaType<RegExpPath>();
    qRegisterMetaType<SearchParams>();

    QApplication a(argc, argv);
    a.setApplicationName(APP_NAME);
    a.setApplicationVersion(APP_VERSION);

    //RegExpPath::test();
    //RegExp::test();

    MainWindow w;
    w.show();

    //SearchCache::testTokenize();
    
    return a.exec();
}
