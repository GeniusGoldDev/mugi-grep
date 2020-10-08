// DO NOT EDIT ** This file was generated by mugicpp ** DO NOT EDIT
#include "searchparams.h"

SearchParams::SearchParams() : mId(-1) {
}
SearchParams::SearchParams(int id, const QString& path, const RegExp& search,
                           const RegExpPath& filter, bool cacheFileList, bool skipBinary)
    : mId(id), mPath(path), mSearch(search), mFilter(filter), mCacheFileList(cacheFileList),
      mSkipBinary(skipBinary) {
}
int SearchParams::id() const {
    return mId;
}
void SearchParams::setId(int value) {
    mId = value;
}
QString SearchParams::path() const {
    return mPath;
}
void SearchParams::setPath(const QString& value) {
    mPath = value;
}
RegExp SearchParams::search() const {
    return mSearch;
}
void SearchParams::setSearch(const RegExp& value) {
    mSearch = value;
}
RegExpPath SearchParams::filter() const {
    return mFilter;
}
void SearchParams::setFilter(const RegExpPath& value) {
    mFilter = value;
}
bool SearchParams::cacheFileList() const {
    return mCacheFileList;
}
void SearchParams::setCacheFileList(bool value) {
    mCacheFileList = value;
}
bool SearchParams::skipBinary() const {
    return mSkipBinary;
}
void SearchParams::setSkipBinary(bool value) {
    mSkipBinary = value;
}
