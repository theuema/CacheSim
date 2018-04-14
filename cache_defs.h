//
// Created by Mario Theuermann on 27.03.18.
//

#ifndef CACHESIM_OFFLINE_CACHE_DEFS_H
#define CACHESIM_OFFLINE_CACHE_DEFS_H

#include "cache_config.h"

struct MemCache;                    // forward declared
typedef struct MemCache MemCache;
struct CacheSet;
typedef struct CacheSet CacheSet;
struct CacheLine;
typedef struct CacheLine CacheLine;

struct MemCache {
    uint32_t size_;
    uint32_t ways_;
    uint32_t kbits_;
    uint32_t nbits_;
    uint32_t lines_;
    uint32_t sets_;
    double hit_latency_;
    double aat_L1_miss;
    uint32_t block_size_;
    CacheLine *cache_line_ptr_;      //direct
    CacheSet *cache_set_ptr_;        //associative
    uint64_t cache_hits_;
    uint64_t cache_misses_;
    uint64_t replacements_;
    uint64_t lru_counter_;
};

/* associative cache mapping */
struct CacheSet {
    uint32_t set_lines_;
    CacheLine *set_line_ptr_;
};

struct CacheLine {
    bool valid_;
    size_t tag_;
    uint8_t line_data_[CACHE_BLOCK_SIZE];
    uint64_t accessed_;
};

#endif //CACHESIM_OFFLINE_CACHE_DEFS_H
