#include "QuotientFilter.h"
#include <iostream>


QuotientFilter::QuotientFilter(size_t numBuckets, 
                               std::shared_ptr<HashFamily> family) {
  fp = family->get();
  std::vector<int> temp(numBuckets, 0);
  buckets = temp; 
  numBucks = numBuckets;
  r = 16;
  q = 16;
  std::vector<bool> stat_temp(3*numBuckets, false);
  stat_arr = stat_temp;
}

QuotientFilter::~QuotientFilter() {
  // TODO implement this
}

int QuotientFilter::getqr(int f, bool is_q) const {
  // TODO: Implement this
  int mask = 0xffffffff;
  int ret = 0;
  if(is_q){
      mask = mask << r;
      ret = f & mask;
      ret = (ret >> r);
  }else {
      mask = mask >> (8*sizeof(int) - r);
      ret = f & mask;
  }
  return ret;
}

int QuotientFilter::insert(int data) {
  /* Pseudo Code: 
   *    Scratch that, do what contains 
   *    explains and shift everything
   *    (actually much more similar to
   *    robin hood hashing)
   * */


  int f = fp(data);
  int q_int = getqr(f, true);
  int r_int = getqr(f, false);
  size_t bucket = q_int % numBucks;

  // If bucket empty, place.
  if(!isFilled(bucket)) {
    std::cout<<"No Eviction"<<std::endl;
    buckets[bucket] = r_int;
    set_3_bit(bucket, true, false, false);
    return 1;
  }

  // Now let's scan through run
  std::vector<size_t> cluster_info = scan_left(bucket);
  size_t cluster_start = cluster_info[0]; // Start of the cluster
  size_t runs_before = cluster_info[1];   // Number of runs before the one we want
  bool first_in_run = stat_arr[bucket*3]; // If our run already exists..


  // If cluster_start is > numBucks, then saturated and done inserting...
  if(cluster_start > numBucks)
    return -1;

  // Scan right now
  size_t run_start = scan_right(cluster_start, runs_before);
  bool inserted = false;
  bool last_cont = !first_in_run;
  bool run_over = false;
  size_t started = run_start;

  // After scans, make sure to set connonical bit:
  stat_arr[bucket*3] = true;

  // Loop while still looking at non-empty spots
  while(isFilled(run_start) == true) {
    // If we hit value, return right away
    if(buckets[run_start] == r_int)
      return 1;
    // If slot where r_int goes and have not inserted, evict
    if((buckets[run_start] > r_int) || inserted || first_in_run || run_over){
      int temp = buckets[run_start];
      buckets[run_start] = r_int;
      bool temp_cont = stat_arr[(run_start * 3) + 1];
      if(!inserted){
        std::cout<<"INSERTING INTO: "<<run_start<<std::endl;
        std::cout<<"BUCKET VAL: "<<bucket<<std::endl;
        if(first_in_run) {
          stat_arr[(run_start*3) + 1] = false; // set cont to false
          stat_arr[(run_start*3) + 2] = (bucket != run_start);  // Set shifted if shifted
          temp_cont = false;
        }else {
          stat_arr[(run_start*3) + 1] = true; // set cont to true
          stat_arr[(run_start*3) + 2] = true; // set shifted to true
        }
        inserted = true;
      }else {
        stat_arr[(run_start*3) + 1] = last_cont;
        stat_arr[(run_start*3) + 2] = true;
      }
      last_cont = temp_cont;
      r_int = temp;
    }
    run_start = increment(numBucks, run_start);
    // To catch infinite loops on saturation...
    if(run_start == started)
        return -1;
    if(!run_over)
        run_over = !stat_arr[(run_start*3) + 1]; // If cont false, run is over and we need to insert
  }

  // Insert
  buckets[run_start] = r_int;
  stat_arr[(run_start*3) + 1] = last_cont;
  stat_arr[(run_start*3) + 2] = true;
  return 1;
}


bool QuotientFilter::contains(int data) const {
  int f = fp(data);
  int q_int = getqr(f, true);
  int r_int = getqr(f, false);
  size_t bucket = q_int % numBucks;
  size_t run_start = bucket;
  size_t started = bucket;

  // If connonical slot empty, return false
  if(stat_arr[bucket*3] == false){
      std::cout<<"Connonical slot empty"<<std::endl;
      return false;
  }

  // If we have not shifted, run starts at spot.
  if(stat_arr[(bucket*3) + 2] == true) {
    //std::cout<<"SHIFTED"<<std::endl;
    // Now let's scan through run
    std::vector<size_t> cluster_info = scan_left(bucket);
    size_t cluster_start = cluster_info[0];
    size_t runs_before = cluster_info[1];

    // If cluster_start is > numBucks, then saturated...
    if(cluster_start > numBucks)
        return false;
    std::cout<<"Cluster Start: "<<cluster_start<<std::endl;
    // Scan right now
    run_start = scan_right(cluster_start, runs_before);
    //run_start =cluster_start;
    started = run_start;
  }
  std::cout<<"RUN START: "<<run_start<<std::endl;  
  // Loop while still looking at non-empty spots
  while(isFilled(run_start) == true) {
    // If we hit value, true!!
    if(buckets[run_start] == r_int)
      return true;
    // Because we are in sorted order, stop early if greater
    if(buckets[run_start] > r_int)
      return false;
    run_start = increment(numBucks, run_start);

    // If our run is over, return false
    if(stat_arr[(run_start*3) + 1] != true)
        return false;
    if(started == run_start)
        return false;
  }
  //std::cout<<"Out of loop"<<std::endl;
  return false;
}

bool QuotientFilter::linscan(int data) const{
  int f = fp(data);
  //int q_int = getqr(f, true);
  int r_int = getqr(f, false);
  for(int i = 0; i < numBucks; i++) {
    if(buckets[i] == r_int){
        std::cout<<"Lin scan ind: "<<i<<std::endl;
        return true;
    }
  }
  return false;
}

void QuotientFilter::remove(int data) {
   // TODO implement this
    /* Pseudo Code:
   *    - do same search as contains
   *      and then shift rest of the
   *      cluster forward....
   *
   *
   *
   * */
}


// Below are helper functions for the main algorithms

size_t QuotientFilter::scan_right(size_t ind, size_t runs) const{
  /* This funciton scans the buckets array right
   * decrementing the runs_left variable at each
   * run (ie each time continuation is false).
   * returns the beginning of the runs_left^th
   * run.
   */
  while(runs != 0) {
    if(stat_arr[(ind * 3) + 1] == false) {
      runs -= 1;
    }
    // Only increment if not at target run.
    if(runs != 0){
      ind = increment(numBucks, ind);
    }
  }
  return ind;
}

std::vector<size_t> QuotientFilter::scan_left(size_t ind) const{
  /* This function scans the buckets array
   * left to find the beginning of the cluster
   * the index belongs to.
   */
  size_t cluster_start = ind;
  size_t runs = 0;
  // Loop non-empty
  while(isFilled(cluster_start)) {
      if(stat_arr[(cluster_start *3)] == true)
          runs += 1;
      cluster_start = decrement(cluster_start);
      if(cluster_start == ind) {
        return {numBucks + 1, runs};
      }
  }
  // Fence post.  Need to re increment cluster to get start
  cluster_start = increment(numBucks, cluster_start);

  // Subtract one off of runs to account for your own run
  return {cluster_start, runs - 1};
}

bool QuotientFilter::isFilled(size_t ind) const{
  /* This funciton returns whether or not a given
   * index is filled (true) or empty (false)
   */
  return stat_arr[ind*3] || stat_arr[(ind * 3) +1] || stat_arr[(ind*3) +2];
}


size_t QuotientFilter::decrement(size_t bucket) const{
  /* This function takes in a bucket index and
   * decrements it, accounting for wrap around.
   */
    
  if(bucket == 0)
    return numBucks - 1;
  return bucket - 1;
}

void QuotientFilter::set_3_bit(size_t ind, bool occ,
                        bool cont, bool shift) {
  /* This function takes in an index into the 
   * buckets array and sets the corresponding
   * 3 bit bucket state with the 3 bool values
   * provided.  Occ is occupied, cont is continuation,
   * and shfit is shifted.
   */

  stat_arr[ind * 3] = occ;
  stat_arr[(ind * 3) + 1] = cont;
  stat_arr[(ind * 3) + 2] = shift;
}


size_t QuotientFilter::increment(size_t numBucks, size_t bucket) const {
  if(bucket >= numBucks - 1)
    return 0;
  return bucket + 1;
}
