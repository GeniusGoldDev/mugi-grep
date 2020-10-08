// DO NOT EDIT ** This file was generated by mugicpp ** DO NOT EDIT
#ifndef SEARCHHITS_H
#define SEARCHHITS_H
#include "regexp.h"
#include "searchhit.h"
#include <QList>
#include <QMetaType>

class SearchHits {

public:
    SearchHits();
    SearchHits(const RegExp& search, const QList<SearchHit>& hits, int total, int complete);
    void append(const SearchHits& hits);
    void append(const SearchHit& hit);
    int size() const;
    SearchHit& hit(int index);
    void read(int before, int after);
    RegExp search() const;
    void setSearch(const RegExp& value);
    QList<SearchHit> hits() const;
    void setHits(const QList<SearchHit>& value);
    int total() const;
    void setTotal(int value);
    int complete() const;
    void setComplete(int value);

protected:
    RegExp mSearch;
    QList<SearchHit> mHits;
    int mTotal;
    int mComplete;
};

Q_DECLARE_METATYPE(SearchHits)

#endif // SEARCHHITS_H
