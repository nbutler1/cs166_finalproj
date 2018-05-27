#include "BucketsTable.h"
#include <stddef.h>
#include <stdlib.h>
#include <set>
//struct entry{
//    int f;
//    int empty;
//}; 

BucketsTable::BucketsTable(size_t numBuckets) {
    num_buckets = numBuckets;
    num_elems = 0;
    entries = new std::set<int>;
}

BucketsTable::~BucketsTable() {
    // TODO Implement this
    delete entries;
}

bool BucketsTable::has(int f) {
    // TODO: Implement this
    auto search = entries->find(f);
    return (search != entries->end());
}

bool BucketsTable::full() {
    return (num_elems >= num_buckets);
}

void BucketsTable::add(int f) {
    // TODO: implement this
    entries->insert(f);
    num_elems += 1;
}

int BucketsTable::evict_rand_and_replace(int f) {
    // TODO: Implement this
    int ind = rand() % (num_elems - 1);
    int fp = *std::next(entries->begin(), ind);
    entries->erase(std::next(entries->begin(), ind));
    entries->insert(f);
    return fp;
}

bool BucketsTable::check_and_remove(int f) {
    // TODO: Implement this
    auto search = entries->find(f);
    if(search != entries->end()) {
        entries->erase(search);
        num_elems -= 1;
        return true;
    }
    return false;
}
