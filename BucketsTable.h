#ifndef BucketsTable_Included
#define BucketsTable_Included

#include <stddef.h>
#include <set>

class BucketsTable{
    public:
        /**
         * Constructor
         */
        BucketsTable(size_t numBuckets);

        /**
         * Deconstructor
         */
        ~BucketsTable();

        /**
         * Function returns true if table has finger print
         * false otherwise. 
         */
        bool has(int f);

        /**
         * Function returns if buckets table is full
         */
        bool full();

        /**
         * Function adds given finger print
         */
        void add(int f);

        /**
         * Function picks a random element, replaces it 
         * with the given finger print, and returns the
         * evicted finger print
         */
        int evict_rand_and_replace(int f);

        /**
         * Function checks if given element is in the table,
         * removes it if it is, and returns whether or not
         * the element was removed.
         */
        bool check_and_remove(int f);

    private:
        size_t num_elems;
        size_t num_buckets;
        std::set<int>* entries;
        BucketsTable(BucketsTable const &) = delete;
        void operator=(BucketsTable const &) = delete;
};

#endif
