// DO NOT EDIT ** This file was generated by mugicpp ** DO NOT EDIT
#include "searchhits.h"

SearchHits::SearchHits() : mTotal(-1), mComplete(-1) {
}
SearchHits::SearchHits(const RegExp& pattern, const QList<SearchHit>& hits, int total, int complete)
    : mPattern(pattern), mHits(hits), mTotal(total), mComplete(complete) {
}
SearchHits::SearchHits(const RegExp& pattern, const QList<SearchHit>& hits)
    : mPattern(pattern), mHits(hits), mTotal(-1), mComplete(-1) {
}
SearchHits::SearchHits(const RegExp& pattern) : mPattern(pattern), mTotal(-1), mComplete(-1) {
}
RegExp SearchHits::pattern() const {
    return mPattern;
}
void SearchHits::setPattern(const RegExp& value) {
    mPattern = value;
}
QList<SearchHit> SearchHits::hits() const {
    return mHits;
}
void SearchHits::setHits(const QList<SearchHit>& value) {
    mHits = value;
}
int SearchHits::total() const {
    return mTotal;
}
void SearchHits::setTotal(int value) {
    mTotal = value;
}
int SearchHits::complete() const {
    return mComplete;
}
void SearchHits::setComplete(int value) {
    mComplete = value;
}
void SearchHits::append(const SearchHits& hits) {
    mPattern = hits.pattern();
    mHits.append(hits.hits());
}
void SearchHits::append(const SearchHit& hit) {
    mHits.append(hit);
}
int SearchHits::size() const {
    return mHits.size();
}
bool SearchHits::isEmpty() const {
    return mHits.isEmpty();
}
SearchHit SearchHits::hit(int index) const {
    return mHits[index];
}
SearchHits SearchHits::mid(int index) {
    return SearchHits(mPattern, mHits.mid(index));
}
void SearchHits::read(int before, int after) {
    for (int i = 0; i < mHits.size(); i++) {
        mHits[i].read(before, after);
    }
}
void SearchHits::clear() {
    mHits.clear();
}
