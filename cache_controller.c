//
// Created by theuema on 11/15/17.
//

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <assert.h>
#include "cache_controller.h"

/*CacheSim  */
MemCache *cache;

/* function to return Cache* to main for statistical evaluation*/
MemCache *getCacheInstance() { return cache; }

// constructor (OOP style);
void MemCache__init(MemCache *self, uint32_t size, uint32_t ways, CacheLine *line_ptr, CacheSet *set_ptr,
                    uint32_t nbits, uint32_t kbits, uint32_t lines, uint32_t sets, double aat) {
    self->size_ = size;
    self->ways_ = ways;
    self->kbits_ = kbits;
    self->nbits_ = nbits;
    self->lines_ = lines;
    self->sets_ = sets;
    self->cache_line_ptr_ = line_ptr;
    self->cache_set_ptr_ = set_ptr;
    self->cache_hits_ = 0;
    self->cache_misses_ = 0;
    self->replacements_ = 0;
    self->lru_counter_ = 0;
    self->hit_latency_ = L1_HIT_LATENCY;
    self->aat_L1_miss = aat;

}

// cache allocation & initialization
void MemCache__create(void) {
    uint32_t size = CACHE_SIZE * 1024;
    // Average access time (AAT)
    // approximated latency on L1 miss with x-Level cache hierachy
    double aat_L1_miss = L2_HIT_LATENCY + L2_MISSRATE * (L3_HIT_LATENCY + L3_MISSRATE * MEM_LATENCY);
    cache = (MemCache *) malloc(sizeof(MemCache));
    cache->block_size_ = CACHE_BLOCK_SIZE;
    double kbits_d = log(cache->block_size_) / log(2); // offset for addressing each byte in cache line
    uint32_t kbits = (uint32_t) kbits_d;
    uint32_t nbits;
    uint32_t lines;
    uint32_t sets;
    /* direct cache */
    CacheLine *line_ptr;
    /* associative cache */
    CacheSet *set_ptr;
    uint32_t ways;

    /* allocate cache here;
     *
     * #direct cache
     *  cache line formula: (capacity kb * 1024) / (sizeof(cache_line)); // 64bytes;
     *  cache line: 128kb -> 2048  lines
     *  cache line: 512kb -> 8192  lines
     *  cache line: 2MB   -> 32786 lines
     *
     * #associated cache
     *  cache set formula: ((kb * 1024) / (sizeof(cache_line)) / association); association = ways;
     *  8MB   -> 131.072 lines / 16 ways = 8192 sets
     */

    /* direct cache mapping */
    if (DIRECT_CACHE) {
        ways = 0;
        sets = 0;
        set_ptr = NULL;
        double nbits_d = log(size / cache->block_size_) / log(2); // size of bits needed for line index
        nbits = (uint32_t) nbits_d;
        lines = size / cache->block_size_;
        line_ptr = (CacheLine *) malloc(lines * (sizeof(CacheLine)));

        // init members of allocated lines
        CacheLine *curr_line_ptr;
        for (uint32_t i = 0; i < lines; ++i) {
            curr_line_ptr = line_ptr + i;
            curr_line_ptr->tag_ = 0;
            curr_line_ptr->valid_ = 0;
            curr_line_ptr->accessed_ = 0;
        }
    } else {
        /* associative cache mapping */
        line_ptr = NULL;
        lines = 0;
        ways = CACHE_WAYS;
        assert(ways);
        double nbits_d = log((size / cache->block_size_) / ways) / log(2); // size of bits needed for set index
        nbits = (uint32_t) nbits_d;
        sets = (size / cache->block_size_) / ways;
        set_ptr = (CacheSet *) malloc(sets * (sizeof(CacheSet)));

        // init & alloc members
        CacheSet *curr_set_ptr;
        for (uint32_t i = 0; i < sets; ++i) {
            curr_set_ptr = set_ptr + i;
            curr_set_ptr->set_line_ptr_ = (CacheLine *) malloc(ways * (sizeof(CacheLine)));
            curr_set_ptr->set_lines_ = ways;

            CacheLine *curr_line_ptr;
            for (uint32_t n = 0; n < curr_set_ptr->set_lines_; ++n) {
                curr_line_ptr = curr_set_ptr->set_line_ptr_ + n;
                curr_line_ptr->tag_ = 0;
                curr_line_ptr->valid_ = 0;
                curr_line_ptr->accessed_ = 0;
            }
        }
    }
    MemCache__init(cache, size, ways, line_ptr, set_ptr, nbits, kbits, lines, sets, aat_L1_miss);
    printf("[Cache controller] Succesfully created cache.\n\n");
}

void MemCache__free(void) {
    /* direct cache mapping */
    if (DIRECT_CACHE) {
        free(cache->cache_line_ptr_);
        free(cache);
    } else { /* associative cache mapping */
        CacheSet *curr_set_ptr;
        for (uint32_t i = 0; i < cache->sets_; i++) {
            curr_set_ptr = cache->cache_set_ptr_ + i;
            free(curr_set_ptr->set_line_ptr_);
        }
        free(cache->cache_set_ptr_);
        free(cache);
    }
    printf("[Cache controller] Succesfully freed cache.\n\n");
}

void lru_replace(CacheLine *cache_line, size_t addr_tag){
    cache_line->tag_ = addr_tag;
    cache_line->accessed_ = cache->lru_counter_;
    cache->replacements_++;
}

void random_replace(CacheLine *cache_line, size_t addr_tag) {
    cache_line->tag_ = addr_tag;
    cache->replacements_++;
}

void direct_cache_miss(unsigned size, bool valid_bit, CacheLine *cache_line, size_t addr_tag) {
    // store TAG to CACHE and set valid bit
    cache_line->tag_ = addr_tag;
    if (valid_bit)
        cache->replacements_++;
    else
        cache_line->valid_ = true;

    cache->cache_misses_++;
}

void associative_cache_miss(unsigned size, bool replacement,
                            CacheLine *cache_line, size_t addr_tag) {

    // no replacement because line never used before; just store tag;
    if (!replacement) {
        cache_line->valid_ = true;
        cache_line->tag_ = addr_tag;
        goto miss_out;
    }

    /* replacement algorithm */
#if LRU
    lru_replace(cache_line, addr_tag);
#elif RANDOM
    random_replace(cache_line, addr_tag);
#endif

    miss_out:
    cache->cache_misses_++;
}

void check_hit_miss(size_t addr, unsigned size) {
    size_t addr_tag = addr >> (cache->kbits_ + cache->nbits_);
    CacheLine *cache_line;

    if (!DIRECT_CACHE)
        goto associative;

/*****************************/
/* direct cache mapping */
/*****************************/
    // get actual cache line index by cutting off klast bits & TAG
    uint64_t cache_line_index;

    if (cache->nbits_ == 0)      // workaround: not allowed to shift zero nbits_
        cache_line_index = 0;
    else
        cache_line_index = (((addr >> cache->kbits_) << (sizeof(addr) * 8 - cache->nbits_))
                >> (sizeof(addr) * 8 - cache->nbits_));

    cache_line = cache->cache_line_ptr_ + cache_line_index;

    if (cache_line->valid_ && cache_line->tag_ == addr_tag) {
        // actual cache line is valid and TAGs are congruent            -> cache hit
        cache->cache_hits_++;
        goto check_done;
    }

    // actual addr_TAG != cache_line_TAG | !valid bit                   -> cache miss
    direct_cache_miss(size, cache_line->valid_, cache_line, addr_tag);
    goto check_done;

/*****************************/
/* associative cache mapping */
/*****************************/
    associative:;
    uint64_t cache_set_index;
    if (cache->nbits_ == 0)      // workaround: not allowed to shift zero nbits_
        cache_set_index = 0;
    else
        cache_set_index = (((addr >> cache->kbits_) << (sizeof(addr) * 8 - cache->nbits_))
                >> (sizeof(addr) * 8 - cache->nbits_));

    CacheSet *cache_set = cache->cache_set_ptr_ + cache_set_index;

    // replacement helper
#if LRU
    cache->lru_counter_++;
#endif
    bool replacement = false;
    CacheLine *evict_cache_line = cache_set->set_line_ptr_;

    // go through all cache set lines of set and compare TAG
    for (uint32_t i = 0; i < cache_set->set_lines_; ++i) {
        cache_line = cache_set->set_line_ptr_ + i;
        if (cache_line->valid_ && cache_line->tag_ == addr_tag) {
            // actual cache line is valid and TAGs are congruent            -> cache hit
            cache->cache_hits_++;
#if LRU
            cache_line->accessed_ = cache->lru_counter_;
#endif
            goto check_done;
        }

        if (!cache_line->valid_) {
            // line never used before                                       -> cache miss
            associative_cache_miss(size, replacement, cache_line, addr_tag);
            goto check_done;
        }

#if LRU
        if(cache_line->accessed_ < evict_cache_line->accessed_)
            evict_cache_line = cache_line;
#elif RANDOM
        if (i + 1 == cache_set->set_lines_) {
            int rand_index = rand() / (RAND_MAX / cache_set->set_lines_ + 1);
            evict_cache_line = cache_set->set_line_ptr_ + rand_index;
        }
#endif
        if (i + 1 == cache_set->set_lines_) {
            // all lines checked - cache miss with replacement              -> cache miss
            replacement = true;
            associative_cache_miss(size, replacement, evict_cache_line, addr_tag);
        }
    }
    check_done:
    return;
}

void cache_flush_all(void) {
    /* direct cache mapping */
    if (DIRECT_CACHE) {
        CacheLine *curr_line_ptr;
        for (uint32_t i = 0; i < cache->lines_; i++) {
            curr_line_ptr = cache->cache_line_ptr_ + i;
            curr_line_ptr->valid_ = false;
        }
    } else { /* associative cache mapping */
        CacheSet *curr_set_ptr;
        for (uint32_t i = 0; i < cache->sets_; i++) {
            curr_set_ptr = cache->cache_set_ptr_ + i;

            CacheLine *curr_line_ptr;
            for (uint8_t n = 0; n < curr_set_ptr->set_lines_; n++) {
                curr_line_ptr = curr_set_ptr->set_line_ptr_ + n;
                curr_line_ptr->valid_ = false;
            }
        }
    }
}
