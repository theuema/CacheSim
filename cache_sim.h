//
// Created by theuema on 11/15/17.
//

#ifndef CACHE_SIM_H
#define CACHE_SIM_H
extern "C" {
#include "cache_controller.h"
}

bool ReadFromFile(std::vector<size_t> &addr_trace, const char *filename);

void PrintSimpleLog(MemCache *cache, vector<size_t> &addr_trace, const string &prefix, const string &time,
                    bool file_write_enabled);

void RecordAccessTime(MemCache *cache, vector<double> &access_t, int64_t &last_hit_count,
                      int64_t &last_miss_count);

void WriteAccessTimeToFile(vector<double> &mean, const string &prefix, const string &time);

string createFileOutputString(const string &prefix, const string &time);

string createTimeString();

#endif //CACHE_SIM_H
