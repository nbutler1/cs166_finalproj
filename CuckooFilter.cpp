#include "CuckooFilter.h"
#include "BucketsTable.h"
#include <math.h>
#include <vector>
#include <functional>
#include <stdlib.h>
#include <iostream>

CuckooFilter::CuckooFilter(size_t arr_size, std::shared_ptr<HashFamily> family) {
  size_t numBuckets = 4; 
  // First set variables  
  numBucks = arr_size;
  h1 = family->get();
  finger_pointer = family->get();
  num_max_cuckoos = 500;
  num_elems = 0;

  // Now set b1 and b2
  std::vector<BucketsTable*> temp(arr_size, NULL);
  std::vector<BucketsTable*> temp1(arr_size, NULL);
  b1 = temp;
  b2 = temp1; 
  for(size_t i = 0; i < arr_size; i++) {
    b1[i] = new BucketsTable(numBuckets);
    b2[i] = new BucketsTable(numBuckets);
  }
}

CuckooFilter::~CuckooFilter() {
  for(size_t i = 0; i < b1.size(); i++){
    delete b1[i];
    delete b2[i];
  }
}

int CuckooFilter::run_cuckoo_loop(int f, int ind1, int ind2) {
  int counter = rand() % 2;
  int ind = (counter % 2 == 0) ? ind2 : ind1;
  for(size_t i = 0; i < num_max_cuckoos; i++) {
    if(counter % 2 == 0) {
        if(!b1[ind]->full()){
            b1[ind]->add(f);
            return 1;
        }
        f = b1[ind]->evict_rand_and_replace(f);
    }else {
        if(!b2[ind]->full()){
            b2[ind]->add(f);
            return 1;
        }
        f = b2[ind]->evict_rand_and_replace(f);
    }
    ind = ind ^ h1(f);
    counter += 1;
  }
  return -1;
}

int CuckooFilter::insert(int data) {
  int f = finger_pointer(data);
  int ind1 = h1(data) % numBucks;
  int ind2 = (ind1 ^ f) % numBucks;
  //std::cout<<"Ind 1: "<<ind1<<" Ind 2: "<<ind2<<std::endl;
  if(b1[ind1]->has(f) || b2[ind2]->has(f))
    return 1;
  if(!b1[ind1]->full()){
    b1[ind1]->add(f);
    return 1;
  }
  if(!b2[ind2]->full()){
    b2[ind2]->add(f);
    return 1;
  }
  return run_cuckoo_loop(f, ind1, ind2);
}

bool CuckooFilter::contains(int data) const {
  int f = finger_pointer(data);
  int ind1 = h1(f) % numBucks;
  int ind2 = (ind1 ^ f) % numBucks;
  if(b1[ind1]->has(f) || b2[ind2]->has(f))
      return true;
  return false;
}

void CuckooFilter::remove(int data) {
  int f = finger_pointer(data);
  int ind1 = h1(f);
  int ind2 = ind1 ^ f;
  if(b1[ind1]->check_and_remove(f))
      return;
  b2[ind2]->check_and_remove(f);
}
