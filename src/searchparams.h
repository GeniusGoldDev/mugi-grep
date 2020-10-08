// DO NOT EDIT ** This file was generated by mugicpp ** DO NOT EDIT
#ifndef SEARCHPARAMS_H
#define SEARCHPARAMS_H
#include "regexp.h"
#include "regexppath.h"
#include <QMetaType>
#include <QString>

class SearchParams {

public:
    SearchParams();
    SearchParams(int id, const QString& path, const RegExp& search, const RegExpPath& filter,
                 bool cacheFileList, bool skipBinary);
    int id() const;
    void setId(int value);
    QString path() const;
    void setPath(const QString& value);
    RegExp search() const;
    void setSearch(const RegExp& value);
    RegExpPath filter() const;
    void setFilter(const RegExpPath& value);
    bool cacheFileList() const;
    void setCacheFileList(bool value);
    bool skipBinary() const;
    void setSkipBinary(bool value);

protected:
    int mId;
    QString mPath;
    RegExp mSearch;
    RegExpPath mFilter;
    bool mCacheFileList;
    bool mSkipBinary;
};

Q_DECLARE_METATYPE(SearchParams)

#endif // SEARCHPARAMS_H
