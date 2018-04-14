#ifndef CACHE_CONFIG_H
#define CACHE_CONFIG_H

/*****************************/
/* L1 cache properties */
/*****************************/
#define CACHE_BLOCK_SIZE 64
#define CACHE_SIZE 32
#define CACHE_WAYS 8
#define L1_HIT_LATENCY 1
#define DIRECT_CACHE 0

/* choose replacement algorithm */
#define RANDOM 0
#define LRU 1

/*****************************/
/* simulated L2/L3/MEM properties */
/*****************************/
#define MEM_LATENCY 80
#define L2_MISSRATE 0.01
#define L3_MISSRATE 0.002
#define L2_HIT_LATENCY 4
#define L3_HIT_LATENCY 10

#endif //CACHE_CONFIG_H
