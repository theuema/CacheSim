//
// Created by theuema on 11/15/17.
//

#ifndef CACHE_CONTROLLER_H
#define CACHE_CONTROLLER_H

#include "cache_defs.h"

void MemCache__init(MemCache *self, uint32_t size, uint8_t ways, CacheLine *line_ptr, CacheSet *set_ptr,
                    uint8_t nbits, uint8_t kbits, uint32_t lines, uint32_t sets);

void MemCache__create(void);

void MemCache__free(void);

void check_hit_miss(size_t addr, unsigned size);

void direct_cache_miss(unsigned size, bool valid_bit, CacheLine *cache_line, size_t addr_tag);

void associative_cache_miss(unsigned size, bool replacement,
                            CacheLine *cache_line, size_t addr_tag);

MemCache *getCacheInstance();

void lru_replace(CacheLine *cache_line, size_t addr_tag);

void random_replace(CacheLine *cache_line, size_t addr_tag);

void cache_flush_all(void);

#endif //CACHE_CONTROLLER_H
