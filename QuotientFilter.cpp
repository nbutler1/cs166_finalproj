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
  num_elems = 0;
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
  //std::cout<<"BUCKET VALUE: "<<bucket<<std::endl;

  // If bucket empty, place.
  if(!isFilled(bucket)) {
    num_elems += 1;
    //std::cout<<"No Eviction"<<std::endl;
    //std::cout<<"INSERTING INTO: "<<bucket<<std::endl;
    buckets[bucket] = r_int;
    set_3_bit(bucket, true, false, false);
    return 1;
  }
  
  size_t run_start = bucket;
  bool first_in_run = !stat_arr[bucket*3];
  
  if(!isClusterStart(bucket)) { 
    // Now let's scan through run
    std::vector<size_t> cluster_info = scan_left(bucket);
    size_t cluster_start = cluster_info[0]; // Start of the cluster
    size_t runs_before = cluster_info[1];   // Number of runs before the one we want
    //std::cout<<"AFTER SCAN LEFT: "<<std::endl;
    //std::cout<<"CLUSTER START: "<<cluster_start<<std::endl;
    //std::cout<<"RUNS BEFORE: "<<runs_before<<std::endl;
    //std::cout<<"BUCKET: "<<bucket<<std::endl;
  
    // If cluster_start is > numBucks, then saturated and done inserting...
    if(cluster_start > numBucks)
        return -1;

    // Scan right now
    run_start = scan_right(cluster_start, runs_before);
  }
  bool inserted = false;
  bool last_cont = !first_in_run;
  bool run_over = false;
  size_t started = run_start;
  
  // After scans, make sure to set connonical bit:
  stat_arr[bucket*3] = true;
  //std::cout<<"RUN START: "<<run_start<<std::endl;
  while(isFilled(run_start) == true) {
    if(!inserted) {
        int temp = buckets[run_start];
        inserted = true;
        // If we are the first element in the run
        if(first_in_run) {
    //        std::cout<<"INSERTING INTO: "<<run_start<<std::endl;
            stat_arr[(run_start*3) + 1] = false;
            stat_arr[(run_start*3) + 2] = (bucket != run_start);
            buckets[run_start] = r_int;
            r_int = temp;
            last_cont = false;
        }else if(run_over) { 
            
      //      std::cout<<"INSERTING INTO: "<<run_start<<std::endl;
            stat_arr[(run_start*3) + 1] = true;
            stat_arr[(run_start*3) + 2] = true;
            buckets[run_start] = r_int;
            r_int = temp;
            last_cont = false;
        }else if((buckets[run_start] > r_int)) {
        //    std::cout<<"INSERTING INTO: "<<run_start<<std::endl;
            //stat_arr[(run_start*3) + 1] = true;
            //stat_arr[(run_start*3) + 2] = (bucket != run_sta;
            buckets[run_start] = r_int;
            r_int = temp;
            last_cont = true;
        } else if(buckets[run_start] == r_int) {
            return 1;
        }else {
            inserted = false;
        }
    } else {
        // save state
        int temp = buckets[run_start];
        bool temp_cont = stat_arr[(run_start*3) + 1];
        stat_arr[(run_start*3) + 1] = last_cont;
        stat_arr[(run_start*3) + 2] = true;
        // shift back
        buckets[run_start] = r_int;
        r_int = temp;
        last_cont = temp_cont;
    }
    run_start = increment(numBucks, run_start);
    if(run_start == started)
        return -1;
    if(stat_arr[(run_start*3) + 1] == false)
        run_over = true;
  }
  
  // Loop while still looking at non-empty spots
  /*while(isFilled(run_start) == true) {
    // If slot where r_int goes and have not inserted, evict
    if((buckets[run_start] > r_int) || inserted || first_in_run || run_over){
      int temp = buckets[run_start];
      buckets[run_start] = r_int;
      bool temp_cont = stat_arr[(run_start * 3) + 1];
      if(!inserted){
        if(isClusterStart(run_start) || first_in_run) {
          std::cout<<"HERE"<<std::endl;
          if(first_in_run || run_start == started){
            stat_arr[(run_start*3) + 1] = false; // set cont to false
          }else{
            stat_arr[(run_start*3) + 1] = true;
          }
            
          stat_arr[(run_start*3) + 2] = (bucket != run_start);  // Set shifted if shifted
          temp_cont = !first_in_run;
        }else {
          stat_arr[(run_start*3) + 1] = temp_cont; // set cont to true
          stat_arr[(run_start*3) + 2] = true; // set shifted to true
        }
        if(run_over)
          temp_cont = false;
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
    //if(run_start == started)
    //    return -1;
    */  
//}
  // Insert
  buckets[run_start] = r_int;
  stat_arr[(run_start*3) + 1] = last_cont;
  if(inserted){
    stat_arr[(run_start*3) + 2] = true;
  } else {
    stat_arr[(run_start*3) + 2] = (bucket != run_start);
  }
    
  num_elems += 1;
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
      //std::cout<<"Connonical slot empty"<<std::endl;
      return false;
  }
  //std::cout<<"num_elems: "<<num_elems<<std::endl;
  if(num_elems == numBucks)
      return linscan(data);

  // If we have not shifted, run starts at spot.
  if(!isClusterStart(bucket)) {
    //std::cout<<"SHIFTED"<<std::endl;
    // Now let's scan through run
    std::vector<size_t> cluster_info = scan_left(bucket);
    size_t cluster_start = cluster_info[0];
    size_t runs_before = cluster_info[1];
    
    // Subtract one off of runs before to account for own run
    //runs_before -= 1;

    // If cluster_start is > numBucks, then saturated...
    if(cluster_start > numBucks){
        //std::cout<<"We are saturated..."<<std::endl;
        return linscan(data);
    }
        //std::cout<<"Cluster Start: "<<cluster_start<<std::endl;
    // Scan right now
    run_start = scan_right(cluster_start, runs_before);
    //run_start =cluster_start;
    started = run_start;
    //std::cout<<"AFTER SCAN LEFT IN CONTAINS: "<<std::endl;
    //std::cout<<"CLUSTER START: "<<cluster_start<<std::endl;
    //std::cout<<"RUNS BEFORE: "<<runs_before<<std::endl;
  }
  //std::cout<<"CONTAINS"<<std::endl;
  //std::cout<<"BUCKET: "<<bucket<<std::endl;
  //std::cout<<"RUN START: "<<run_start<<std::endl;
  //std::cout<<"RUN START: "<<run_start<<std::endl;  
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
  bool ret = false;
  for(int i = 0; i < numBucks; i++) {
    if(buckets[i] == r_int){
        //std::cout<<"Lin scan ind: "<<i<<std::endl;
        ret = true;
    }
    //if(!isFilled(i)){
    //    std::cout<<"INDEX NOT FILLED: "<<i<<std::endl;
    //}
  }

  /*for(int j = 0; j < numBucks; j++) {
    std::string out  = "";
    out.resize(7);
    for(int k = 0; k < 3; k++){
      if(stat_arr[(j*3) + k] == false){
        out.push_back('0');
      }else{
        out.push_back('1');
      }
      out.push_back(' ');
    }
    std::cout<<"Index "<<j<<": ["<<out<<"]"<<std::endl;
  }*/
  return ret;
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
  ind = increment(numBucks, ind);
  while(stat_arr[(ind *3) + 1] != false)
      ind = increment(numBucks, ind);

  // Now increment to end of the run
  return ind;
}

std::vector<size_t> QuotientFilter::scan_left(size_t ind) const{
  /* This function scans the buckets array
   * left to find the beginning of the cluster
   * the index belongs to.
   */
  size_t cluster_start = ind;
  size_t runs = 0;
  if(num_elems == numBucks)
      return {numBucks + 1, runs};
  
  // Loop non-empty
  while(!isClusterStart(cluster_start)) {
      if(stat_arr[(cluster_start *3)] == true)
          runs += 1;
      cluster_start = decrement(cluster_start);
      if(cluster_start == ind) {
        return {numBucks + 1, runs};
      }
  }
  // Fence post.  Need to re increment cluster to get start
  //cluster_start = increment(numBucks, cluster_start);
  if(stat_arr[(ind * 3) + 1] == false)
      runs -= 1;
  // Subtract one off of runs to account for your own run
  return {cluster_start, runs};
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

bool QuotientFilter::isClusterStart(size_t bucket) const {
  return (stat_arr[bucket*3] == true && stat_arr[(bucket*3) + 1] == false
          && stat_arr[(bucket*3) + 2] == false);
}
