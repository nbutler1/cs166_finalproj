#include "BucketsTable.h"
#include <stddef.h>
#include <stdlib.h>
#include <set>
#include <vector>
#include <iostream>
//struct entry{
//    int f;
//    int empty;
//}; 

BucketsTable::BucketsTable(size_t numBuckets) {
    num_buckets = numBuckets;
    num_elems = 0;
    //std::set<int> temp;
    //entries = temp;
}

BucketsTable::~BucketsTable() {
    // TODO Implement this
    //delete entries;
}

bool BucketsTable::has(int f) const{
    // TODO: Implement this
    for(size_t i = 0; i < num_elems; i++) {
      if(entries[i] == f)
          return true;
    }
    return false;
    //std::set<int>::iterator search = entries.find(f);
    //std::cout<<(search != entries.end())<<std::endl;
    //return (std::find(entries.begin(), entries.end(), f) != entries.end());
}

bool BucketsTable::full() {
    return (num_elems >= num_buckets);
}

void BucketsTable::add(int f) {
    // TODO: implement this
    entries.push_back(f);
    num_elems += 1;
}

int BucketsTable::evict_rand_and_replace(int f) {
    // TODO: Implement this
    int ind = rand() % (num_elems - 1);
    //int fp = *std::next(entries.begin(), ind);
    int fp = entries[ind];
    entries[ind] = f;
    //entries.erase(std::next(entries.begin(), ind));
    //entries.insert(f);
    return fp;
}

bool BucketsTable::check_and_remove(int f) {
    // TODO: Implement this
    std::vector<int>::iterator ind = std::find(entries.begin(), entries.end(), f);
    if(ind != entries.end()) {
        entries.erase(ind);
        num_elems -= 1;
        return true;
    }
    return false;
}
