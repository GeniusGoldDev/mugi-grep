#include "searchcache.h"

#include <QSet>
#include <QTextCodec>
#include <QDebug>

#include "html.h"
#include "filereader.h"
#include "lit.h"
#include "worker.h"
#include "utils.h"
#include "coloredline.h"
#include "hunk.h"

QStringList bytesToLines(const QByteArray& bytes) {
    QTextCodec* codec = QTextCodec::codecForName("UTF-8");
    QStringList lines = codec->toUnicode(bytes).split("\n");
    return lines;
}

QByteArray linesToBytes(const QStringList& lines) {
    QTextCodec* codec = QTextCodec::codecForName("UTF-8");
    return codec->fromUnicode(lines.join("\n"));
}

QStringList fileNameLineNumber(bool showFileName, bool showLineNumber, const QString& relativePath, const QString& href, int lineNumber) {
    QStringList cols;
    if (showFileName) {
        cols << Html::anchor(relativePath, href, "violet")
             << Html::span(":", "blue");
    }
    if (showLineNumber) {
        cols << Html::span(QString::number(lineNumber), "green")
        << Html::span(":", "blue");
    }
    return cols;
}

QList<int> getMatched(const QStringList& lines, const RegExp& exp)
{
    QList<int> matched;
    for(int i=0;i<lines.size();i++) {
        if (exp.match(lines[i])) {
            matched << i;
        }
    }
    return matched;
}

QSet<int> getSiblings(const QList<int>& matched, int linesBefore, int linesAfter) {
    QSet<int> siblings;
    int match;
    foreach(match,matched) {
        for (int i=0;i<linesBefore;i++) {
            siblings << match - i - 1;
        }
    }
    foreach(match,matched) {
        for (int i=0;i<linesAfter;i++) {
            siblings << match + i + 1;
        }
    }
    return siblings;
}


void searchLines(const QStringList& lines, const QString& path, const QString& relativePath,
                        const SearchParams& params, SearchHits& hits) {

    QStringList res;
    RegExp exp = params.search();
    QList<int> matched = getMatched(lines, exp);
    if (matched.isEmpty()) {
        return;
    }
    SearchHit hit(path, relativePath, matched);
    hits.append(hit);

#if 0

    QSet<int> siblings = getSiblings(matched, linesBefore, linesAfter);

    static QStringList backgroundColors = {"#ffffff","#ffdfba","#baffc9","#bae1ff","#ffffba","#ffb3ba"};

    bool onlyMatched = params.onlyMatched();
    bool showFileName = params.showFileName();
    bool showLineNumber = params.showLineNumber();

    // pass 2
    for(int i=0;i<lines.length();i++) {

        if (matched.contains(i)) {

            QString line = lines[i];

            QRegularExpression regexp = exp.includeExp();
            QRegularExpressionMatchIterator it = regexp.globalMatch(line);

            QString href = "file:///" + QDir::toNativeSeparators(mPath) + "?line=" + QString::number(i+1);

            if (onlyMatched) {
                // each match on new line
                while(it.hasNext()) {
                    QStringList cols = fileNameLineNumber(showFileName, showLineNumber, mRelativePath, href, i+1);
                    QRegularExpressionMatch m = it.next();
                    ColoredLine coloredLine(line);
                    int jmax = qMin(m.lastCapturedIndex(), backgroundColors.size() - 1);
                    for(int j=1;j<=jmax;j++) {
                        coloredLine.paintBackground(m.capturedStart(j), m.capturedEnd(j), j);
                    }
                    coloredLine.paintForeground(m.capturedStart(), m.capturedEnd(), 1);
                    cols << toHtmlSpans(coloredLine.mid(m.capturedStart(),m.capturedLength()), backgroundColors);
                    res << cols.join("");
                }
            } else {
                // all matches on one line
                QStringList cols = fileNameLineNumber(showFileName, showLineNumber, mRelativePath, href, i+1);
                ColoredLine coloredLine(line);
                while(it.hasNext()) {
                    QRegularExpressionMatch m = it.next();
                    int jmax = qMin(m.lastCapturedIndex(), backgroundColors.size() - 1);
                    for(int j=1;j<=jmax;j++) {
                        coloredLine.paintBackground(m.capturedStart(j), m.capturedEnd(j), j);
                    }
                    coloredLine.paintForeground(m.capturedStart(), m.capturedEnd(), 1);
                }
                cols << toHtmlSpans(coloredLine, backgroundColors);
                res << cols.join("");
            }

        } else if (siblings.contains(i)) {

            QString line = lines[i];
            QString href = "file:///" + QDir::toNativeSeparators(mPath) + "?line=" + QString::number(i+1);
            QStringList cols;
            if (showFileName) {
                cols << Html::anchor(mRelativePath, href, "violet")
                     << Html::span("-", "blue");
            }
            if (showLineNumber) {
                cols << Html::span(QString::number(i+1), "green")
                     << Html::span("-", "blue");
            }
            cols << Html::span(line, "black");
            res << cols.join("");
        }
    }


    return res;
#endif
}

int backreferenceLength(const QString& replacement, int pos) {
    if (replacement[pos] == '\\' && pos + 1 < replacement.size() && replacement[pos+1].isDigit()) {
        int len = 1;
        while(++pos < replacement.size()) {
            if (!replacement[pos].isDigit()) {
                break;
            }
            len++;
        }
        return len;
    }
    return -1;
}

QVariantList tokenize(const QString& replacement) {
    QVariantList result;
    int prev = 0;
    int i = 0;
    while (i < replacement.size()) {
        int len = backreferenceLength(replacement, i);
        if (len > 0) {
            if (i - prev > 0) {
                QString part = replacement.mid(prev,i - prev);
                result.append(part);
            }
            QString ref = replacement.mid(i+1,len-1);
            result.append(ref.toInt());
            prev = i + len;
            i += len;
        } else {
            i++;
        }
    }
    if (prev < replacement.size()) {
        QString part = replacement.mid(prev);
        result.append(part);
    }
    return result;
}

void compare(const QVariantList& e, const QVariantList& a) {
    if (e == a) {
        return;
    }
    QString e_ = "{" + Utils::toStringList(e).join("|") + "}";
    QString a_ = "{" + Utils::toStringList(a).join("|") + "}";
    qDebug() << "not equal, expected: " << e_.toStdString().c_str() << ", actual " << a_.toStdString().c_str();
}

QString sameCase(const QString& repl, const QString& orig) {
    if (orig.toLower() == orig) {
        return repl.toLower();
    } else if (orig.toUpper() == orig) {
        return repl.toUpper();
    }
    return repl;
}

QString withBackreferences(const QRegularExpressionMatch& m, const QVariantList& replacement, bool preserveCase) {
    QStringList result;
    foreach(const QVariant& v, replacement) {
        if (v.type() == QVariant::Int) {
            result.append(m.captured(v.toInt()));
        } else if (v.type() == QVariant::String) {
            result.append(v.toString());
        } else {
            qDebug() << "withBackreferences error:" << v;
        }
    }
    QString result_ = result.join("");
    return preserveCase ? sameCase(result_, m.captured(0)) : result_;
}

#if 0
QStringList replacePreview(const QStringList& lines, const QString& path, const QString& mRelativePath,
                           const SearchParams& params, QList<Replacement>& replacements) {

    QStringList res;

    RegExp exp = params.search();
    QString replacement = params.replace();
    bool preserveCase = params.preserveCase();

    QList<int> matched = getMatched(lines, exp);
    QSet<int> siblings = getSiblings(matched, params.linesBefore(), params.linesAfter());

    if (matched.isEmpty()) {
        return res;
    }

    QRegularExpression rx = exp.includeExp();
    QVariantList replacement_ = tokenize(replacement);
    QString href = "file:///" + QDir::toNativeSeparators(path);
    res << Html::anchor(mRelativePath, href, "violet");

    QStringList oldLines;
    QStringList newLines;

    Hunk hunk;

    QString lightRed = "#FFD9DD";
    QString red = "#fdb8c0";
    QString lightGreen = "#D9FFE3";
    QString green = "#acf2bd";

    for(int i=0;i<lines.size();i++) {
        if (matched.contains(i)) {
            QString line = lines[i];
            QRegularExpressionMatchIterator it = rx.globalMatch(line);
            QStringList oldLine;
            QStringList newLine;
            int prev = 0;
            while(it.hasNext()) {
                QRegularExpressionMatch m = it.next();
                QString s = line.mid(prev,m.capturedStart() - prev);
                oldLine << s;
                newLine << s;

                s = line.mid(m.capturedStart(),m.capturedLength());
                oldLine << s;
                newLine << withBackreferences(m,replacement_,preserveCase);
                prev = m.capturedEnd();
            }

            if (prev < line.size()) {
                oldLine << line.mid(prev);
                newLine << line.mid(prev);
            }

            Q_ASSERT(oldLine.join("") == line);

            QString subj = Html::span("- ","blue",lightRed) + Html::spanZebra(oldLine,"black",lightRed,red);
            QString repl = Html::span("+ ","blue",lightGreen) + Html::spanZebra(newLine,"black",lightGreen,green);

            hunk.replace(i + 1, subj, repl);
            oldLines << oldLine.join("");
            newLines << newLine.join("");
        } else if (siblings.contains(i)) {
            QString subj = Html::span("&nbsp;&nbsp;" + lines[i], "black");
            hunk.context(i + 1, subj);
        } else {
            if (!hunk.isEmpty()) {
                QStringList value = hunk.value();
                res << Html::span(QString("@@ %1,%2 %1,%2 @@").arg(hunk.line()).arg(hunk.count()), "blue")
                    << value;
                hunk = Hunk();
            }
        }
    }

    if (!hunk.isEmpty()) {
        QStringList value = hunk.value();
        res << Html::span(QString("@@ %1,%2 %1,%2 @@").arg(hunk.line()).arg(hunk.count()), "blue")
            << value;
        hunk = Hunk();
    }

    QList<ReplacementLine> replacementLines;
    for(int i=0;i<oldLines.size();i++) {
        replacementLines.append(ReplacementLine(matched[i],oldLines[i],newLines[i]));
    }

    replacements.append(Replacement(path,replacementLines));
    return res;
}
#endif

#if 0
QStringList searchBinary(const QStringList& lines, const QString& path, const QString& relativePath,
                        const SearchParams& params) {
    QStringList res;
    RegExp exp = params.search();
    foreach(const QString& line, lines) {
        if (exp.match(line)) {
            QString href = "file:///" + QDir::toNativeSeparators(path); // todo binary offset?
            QStringList cols;
            cols << Html::span("Binary file ", "black")
                 << Html::anchor(relativePath, href, "violet")
                 << Html::span(" matches", "black");
            res << cols.join("");
            break;
        }
    }
    return res;
}
#endif

void searchLines(const QByteArray& bytes, const QString& path, const QString& relativePath,
                        const SearchParams& params, SearchHits& hits, int* lineCount) {

    QTextCodec* codec = QTextCodec::codecForName("UTF-8");
    QStringList lines = codec->toUnicode(bytes).split("\n");
    *lineCount = lines.size();
    return searchLines(lines, path, relativePath, params, hits);
}

#if 0
QStringList replacePreview(const QByteArray& bytes, const QString& path, const QString& relativePath,
                           const SearchParams& params, int* lineCount, QList<Replacement>& replacements) {

    QTextCodec* codec = QTextCodec::codecForName("UTF-8");
    QStringList lines = codec->toUnicode(bytes).split("\n");
    *lineCount = lines.size();
    return replacePreview(lines, path, relativePath, params, replacements);
}


QStringList searchBinary(const QByteArray& bytes, const QString& path, const QString& relativePath,
                         const SearchParams& params) {

    QTextCodec* codec = QTextCodec::codecForName("UTF-8"); // todo guess or chose encoding
    QStringList lines = codec->toUnicode(bytes).split("\n");
    return searchBinary(lines,path,relativePath,params);
}
#endif

SearchCache::SearchCache() {

}

QPair<int,int> SearchCache::countMatchedFiles(QString path, RegExpPath filter, bool notBinary) {

    QMutexLocker locked(&mMutex);

    //qDebug() << filter << notBinary;

    QStringList allFiles = getAllFiles(path, true);
    int filesFiltered;
    int dirsFiltered;
    // todo optimize
    QStringList files = filterFiles(allFiles,filter,notBinary,&filesFiltered,&dirsFiltered);
    return QPair<int,int>(files.size(),allFiles.size());
}

QStringList SearchCache::getAllFiles(QString path, bool cacheFileList) {
    // todo skip directory entirely if filter matches if not cacheFileList
    QString path_ = QDir(path).absolutePath();
    QStringList allFiles;
    if (cacheFileList && mFileList.contains(path_)) {
        allFiles = mFileList[path_];
    } else {
        QDirIterator it(path, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString path = it.next();
            allFiles.append(path);
        }
        if (cacheFileList) {
            mFileList[path_] = allFiles;
        }
    }
    if (!cacheFileList) {
        mFileList.remove(path_);
    }
    return allFiles;
}

QStringList SearchCache::filterFiles(const QStringList& allFiles, RegExpPath filter, bool notBinary, int* filesFiltered, int* dirsFiltered) {

    int filesFiltered_ = 0;
    int dirsFiltered_ = 0;

    QStringList files;

    foreach(const QString& path, allFiles) {
        if (path.contains("/.git/")) { // todo skip settings
            filesFiltered_++;
            continue;
        }
        if (notBinary && Utils::isBinExt(path)) {
            filesFiltered_++;
            continue;
        }
        if (filter.match(path)) {
            files << path;
        } else {
            filesFiltered_++;
        }
    }

    *filesFiltered = filesFiltered_;
    *dirsFiltered = dirsFiltered_;

    return files;
}

void SearchCache::add(SearchParams params) {

    QMutexLocker locked(&mMutex);

    QString path = params.path();
    bool cacheFileList = params.cacheFileList();
    RegExpPath filter = params.filter();
    bool notBinary = params.skipBinary();

    int searchId = params.id();

    QStringList allFiles = getAllFiles(path, cacheFileList);
    int filesFiltered;
    int dirsFiltered;
    QStringList files = filterFiles(allFiles, filter, notBinary, &filesFiltered, &dirsFiltered);

    SearchData data(files, filesFiltered, dirsFiltered);

    mSearchParams.insert(searchId,params);
    mSearchData.insert(searchId,data);
    mReplacements.insert(searchId,QList<Replacement>());
}

void SearchCache::finish(int searchId) {

    QMutexLocker locked(&mMutex);
    mSearchData.remove(searchId);
}

#if 0
bool SearchCache::isPreview(int searchId) {
    QMutexLocker locked(&mMutex);
    return mSearchParams.contains(searchId) && mSearchParams[searchId].action() == Worker::Preview && mReplacements.contains(searchId);
}
#endif

bool SearchCache::search(int searchId, SearchHits& hits) {

    QMutexLocker locked(&mMutex);

    if (!mSearchParams.contains(searchId)) {
        qDebug() << "!mSearchData.contains(searchId)";
        return true;
    }

    SearchParams& searchParams = mSearchParams[searchId];
    SearchData& searchData = mSearchData[searchId];
    //QList<Replacement>& replacements = mReplacements[searchId];

    hits.setSearch(searchParams.search());

    //int lim = qMin(sd.complete + 100, sd.files.size());

    int lineCount = 0;
    int fileCount = 0;

    for (int i=searchData.filesComplete();i<searchData.filesSize();i++) {
        QString path = searchData.file(i);

        bool binary;
        bool readOk;
        bool tooBig;

        //QByteArray fileData = FileCache::instance()->file(path,sd.notBinary,&binary,&readOk);
        QByteArray fileData = FileReader::read(path,searchParams.skipBinary(),&binary,&readOk,&tooBig);
        QString relPath = Utils::relPath(path,searchParams.path());

        bool ok = true;

        if (!readOk) {
            qDebug() << "!readOk" << path;
            ok = false;
        }

        if (tooBig) { // todo implement line by line search in big files
            ok = false;
        }

        int fileLineCount = 0;

        if (ok) {
            if (binary) {
                //res << searchBinary(fileData, path, relPath, searchParams);
            } else {
                searchLines(fileData, path, relPath, searchParams, hits, &fileLineCount);
            }
        }

        lineCount += fileLineCount;
        fileCount += 1;
        searchData.setFilesComplete(i + 1);

        if (lineCount > 4000 || fileCount > 50) {

            /*complete = searchData.filesComplete();
            total = searchData.filesSize();
            filtered = searchData.filesFiltered();*/

            hits.setComplete(searchData.filesComplete());
            hits.setTotal(searchData.filesSize());
            return false;
        }
    }

    hits.setComplete(searchData.filesComplete());
    hits.setTotal(searchData.filesSize());

    return true;

    /*complete = searchData.filesComplete();
    total = searchData.filesSize();
    filtered = searchData.filesFiltered();
    file = QString();*/
}

void SearchCache::replace(int searchId, int* filesChanged, int* linesChanged, QStringList& notChanged) {
    QMutexLocker locked(&mMutex);
    if (!mReplacements.contains(searchId)) {
        return;
    }

    SearchParams params = mSearchParams[searchId];

    *filesChanged = 0;
    *linesChanged = 0;

    QList<Replacement> replacements = mReplacements[searchId];
    foreach(const Replacement& replacement, replacements) {
        bool readOk, tooBig, binary;
        QString path = replacement.path();
        QStringList lines = bytesToLines(FileReader::read(path,true,&binary,&readOk,&tooBig));
        if (!readOk) {
            qDebug() << "cannot read" << path;
            continue;
        }
        if (tooBig) {
            qDebug() << "too big" << path;
            continue;
        }
        bool ok = true;
        QList<ReplacementLine> replLines = replacement.lines();
        foreach (const ReplacementLine& replLine, replLines) {
            int lineNum = replLine.line();
            if (lines[lineNum] == replLine.oldLine()) {
                lines[lineNum] = replLine.newLine();
            } else {
                qDebug() << lines[lineNum];
                qDebug() << replLine.oldLine();
                ok = false;
            }
        }
        if (ok) {
            *filesChanged += 1;
            *linesChanged += replLines.size();
        } else {
            notChanged << Utils::relPath(path,params.path());
        }

        if (!ok) {
            qDebug() << "file changed" << path;
            continue;
        }
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly)) {
            qDebug() << "canot open file" << path;
            continue;
        }
        file.write(linesToBytes(lines));
        file.close();
    }

    mReplacements.remove(searchId);
}

void SearchCache::testTokenize() {

    qDebug() << "testTokenize() started";

    QString t;
    QVariantList a,e;

    t = "foo\\1bar\\2baz";
    e = {"foo",1,"bar",2,"baz"};
    a = tokenize(t);
    compare(e,a);

    t = "foo\\1bar\\2baz\\3";
    e = {"foo",1,"bar",2,"baz",3};
    a = tokenize(t);
    compare(e,a);

    t = "foo\\1bar\\2baz\\3a";
    e = {"foo",1,"bar",2,"baz",3,"a"};
    a = tokenize(t);
    compare(e,a);

    t = "\\1bar\\2baz\\3";
    e = {1,"bar",2,"baz",3};
    a = tokenize(t);
    compare(e,a);

    t = "\\1bar\\2\\3baz\\52";
    e = {1,"bar",2,3,"baz",52};
    a = tokenize(t);
    compare(e,a);

    qDebug() << "testTokenize() finished";
}

