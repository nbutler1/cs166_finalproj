#ifndef Timing_Included
#define Timing_Included

#include "Hashes.h"
#include "Timer.h"
#include <climits>
#include <random>
#include <tuple>
#include <memory>
#include <initializer_list>
#include <unordered_set>
#include <iostream>
#include <iomanip>

/* The random seed used throughout the run. */
static const size_t kRandomSeed = 138;

/* Multiplier used to determine the maximum integer that can be queried on an
 * operation. Higher numbers make for more likely misses in lookups.
 */
static const size_t kSpread = 4;

static const size_t bucket_val = 4;

/**
 * Gather timing information for performing a certain number of actions.
 * The elements used are provided by the given generator.
 */
template <typename F, typename HT>
std::tuple<double, double> timeGenerator(double loadFactor, 
                                         std::shared_ptr<HashFamily> family, F& gen, size_t numActions) {
  std::default_random_engine engine(kRandomSeed);
  
  HT table(numActions + 2, family); // The +2 term ensures that cuckoo hashing rounds the right way.

  Timer insertTimer, queryTimer;
  
  for (size_t i = 0; i < numActions * loadFactor; ++i) {
    int value = gen(engine);
    
    insertTimer.start();
    table.insert(value);
    insertTimer.stop();
  }
  
  for (size_t i = 0; i < numActions; i++) {
    int value = gen(engine);
    
    queryTimer.start();
    table.contains(value);
    queryTimer.stop();
  }
  
  return std::make_tuple(insertTimer.elapsed() / numActions,
                         queryTimer.elapsed()  / numActions);
}


/**
 * Gather timing information for performing a certain number of actions,
 * agnostic to the number of buckets in the underlying structure.
 * The elements used are selected uninformly at random from all integers.
 */
template <typename HT>
std::tuple<double, double> timeAbsolute(double loadFactor, std::shared_ptr<HashFamily> family, 
                                        size_t numActions) {
  auto gen = std::uniform_int_distribution<int>(0, numActions * kSpread);
  return timeGenerator<decltype(gen), HT>(loadFactor, family, gen, numActions);
}

/**
 * Gather timing information for performing 1,000 actions.
 * Returns a pair: (average insert time, average query time).
 */
template <typename HT>
std::tuple<double, double> time1k(double loadFactor, std::shared_ptr<HashFamily> family) {
  return timeAbsolute<HT>(loadFactor, family, 1000);
}

/**
 * Gather timing information for performing 100,000 actions.
 * Returns a pair: (average insert time, average query time).
 */
template <typename HT>
std::tuple<double, double> time100k(double loadFactor, std::shared_ptr<HashFamily> family) {
  return timeAbsolute<HT>(loadFactor, family, 100000);
}

/**
 * Print timing information
 */
template<std::tuple<double, double> timing_func(double, std::shared_ptr<HashFamily>)>
void report(double loadFactor, std::shared_ptr<HashFamily> family){
  auto times = timing_func(loadFactor, family);
  std::cout << "    Insertion: " << std::fixed << std::setw(8) << std::setprecision(2) 
            << std::get<0>(times) << " ns / op" << std::endl;
  std::cout << "    Query:     " << std::fixed << std::setw(8) << std::setprecision(2) 
            << std::get<1>(times) << " ns / op" << std::endl;
}

template <typename HT>
void doAllReports(std::shared_ptr<HashFamily> family, double loadFactor) {
  report<time100k<HT>>(loadFactor, family);
}

template <typename HT>
void doAllReports(std::initializer_list<std::shared_ptr<HashFamily>> factories, std::initializer_list<double> loadFactors) {
  for (auto family : factories) {
    std::cout << "=== " << family->name() << " ===" << std::endl;
    for (auto loadFactor : loadFactors) {
      std::cout << "  --- Load Factor: " << std::fixed << std::setw(8) << std::setprecision(5) << loadFactor << " ---" << std::endl;
      doAllReports<HT>(family, loadFactor);
    }
  }
}


/**
 * Check correctness, using C++'s unordered_set type as an oracle
 */
template <typename HT>
bool checkCorrectness(size_t buckets, std::shared_ptr<HashFamily> family, size_t numActions) {
  std::default_random_engine engine;
  engine.seed(kRandomSeed);
  auto gen = std::uniform_int_distribution<int>(0, INT_MAX);//numActions * kSpread);
  HT table(buckets, family);
  std::unordered_set<int> reference;
  std::cout<<"FUCKING HERE"<<std::endl;
  
  double total = 0;
  double true_negs = 0;
  double false_pos = 0;
  std::cout<<"Num Buckets: "<<buckets<<std::endl;
  while(true) {
    //if((int)total % 10000 == 0)
    //  std::cout<<"Iter Num: "<<total<<std::endl;
    int value = gen(engine);
    //if(reference.count(value) > 0){
     // std::cout<<"ALREADY SEEN"<<std::endl;
     // return true;
    //}
    //std::cout<<"INSERTING"<<std::endl;
    int val = table.insert(value);
    if(val == -1){
        break;
    }
    reference.insert(value);
    //std::cout<<"CHECKING"<<std::endl;
    if ((reference.count(value) > 0) && !table.contains(value)) {
      true_negs += 1;
      //std::cout<<"True Neg.  Value: "<<table.contains(value)<<std::endl;
    }
//    value = gen(engine);
    if((reference.count(value) <= 0) && table.contains(value)) {
      false_pos += 1;
    }
    total += 1;
  }
  std::cout<<"Filter Full After "<< total << " Elems."<<std::endl;
  std::cout<<"Filter had "<<false_pos<<" false positive and "<<true_negs<<" true negatives."<<std::endl;
  std::cout<<"False Positive Rate: "<<(false_pos/ total)<<std::endl;
  std::cout<<"True Negative  Rate: "<<(true_negs/ total)<<std::endl;
  return true;
}

template <typename HT>
bool checkCorrectness(std::initializer_list<std::tuple<int, std::shared_ptr<HashFamily>, int>> params) {
  for (auto param : params) {
    if (!checkCorrectness<HT>(std::get<0>(param), std::get<1>(param), std::get<2>(param))) {
      return false;
    }
  }
  return true;
}

template <typename HT>
bool checkCorrectness(std::initializer_list<std::shared_ptr<HashFamily>> families) {
  for (auto family: families) {
    if (!checkCorrectness<HT>({
          std::make_tuple(12, family, 5),
            std::make_tuple(120, family, 50),
            std::make_tuple(12000, family, 5000)
            })) {
      return false;
    }
  }
  return true;
}

#endif
