#ifndef QuotientFilter_Included
#define QuotientFilter_Included

#include "Hashes.h"
#include <vector>
class QuotientFilter {
public:
  /**
   * Constructs a new Quotient Filter with the specified number of buckets,
   * using hash functions drawn from the indicated family of hash functions.
   */
  QuotientFilter(size_t numBuckets, std::shared_ptr<HashFamily> family);
  
  /**
   * Cleans up all memory allocated by this filter.
   */
  ~QuotientFilter();
 
  int getqr(int f, bool is_q) const;

  /**
   * Inserts the specified element into filter. If the element already
   * exists, this operation is a no-op.
   */
  int insert(int data);


  void set_3_bit(size_t ind, bool occ, bool cont, bool shift);
  
  /**
   * Returns whether the specified key is contained in the filter.
   */
  bool contains(int key) const;
  
  /**
   * Removes the specified element from the filter. If the element is not
   * present in the hash table, this operation is a no-op.
   *
   * You should implement this operation using tombstone deletion - replace the
   * key to remove with a special "tombstone" value indicating that something
   * that was stored here has since been removed.
   */
  void remove(int key);
  
  size_t decrement(size_t bucket) const;
  std::vector<size_t> scan_left(size_t ind) const;
  bool isFilled(size_t ind) const;
  size_t scan_right(size_t ind, size_t runs) const;
  size_t increment(size_t numBucks, size_t bucket) const;
private:
  /* Fun with C++: these next two lines disable implicitly-generated copy
   * functions that would otherwise cause weird errors if you tried to
   * implicitly copy an object of this type. You don't need to touch these
   * lines.
   */
  // Array of buckets containing data
  std::vector<int> buckets;
  size_t numBucks;
  HashFunction fp;
  int r; // remainder
  int q; // quotient
  std::vector<bool> stat_arr; // Use Bool array cause it only stores one bit 
  // Only caviate with bitsets is all of them 
  QuotientFilter(QuotientFilter const &) = delete;
  void operator=(QuotientFilter const &) = delete;
};

#endif
