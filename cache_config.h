#ifndef CACHE_CONFIG_H
#define CACHE_CONFIG_H

/*****************************/
/* cache properties */
/*****************************/

#define CACHE_BLOCK_SIZE 64
#define CACHE_SIZE 32
#define CACHE_WAYS 8
#define MISS_LATENCY 62
#define HIT_LATENCY 1
#define DIRECT_CACHE 0

/* choose replacement algorithm */
#define RANDOM 1

#endif //CACHE_CONFIG_H
