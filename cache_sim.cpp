//
// Created by theuema on 03/26/18.
//

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <assert.h>

using namespace std;

#include "cache_sim.h"

/* reads a memory trace file of form:
 * W 0x7fd7bd2bfc00
 * R 0x7fd7bd2bfc37 */
bool ReadFromFile(vector<size_t> &addr_trace, const char *filename) {
    string s;
    string delim = " ";

    ifstream mem_trace(filename);
    if (mem_trace.is_open()) {
        while (getline(mem_trace, s)) {
            size_t pos = s.find(delim);
            size_t addr;
            string token;
            stringstream ss;
            token = s.substr(0, pos);
            // use data read/write for CacheSim.
            if (token == "W" || token == "R") {
                s.erase(0, pos + delim.length());
            } else // r/w data memory operation allowed only right now
                assert(false);

            ss << s;
            ss >> hex >> addr;
            if (addr)
                addr_trace.push_back(addr);
            else // file read problem
                assert(false);
        }
        goto file_read_finished;
    }
    cout << "Unable to open file." << endl << "CacheSim stopped." << endl;
    return false;

    file_read_finished:
    mem_trace.close();
    if (addr_trace.empty()) {
        cout << "Something wrong with the trace file (delimiter?)." << endl
             << "addr_trace vector size: " << addr_trace.size() << endl
             << "CacheSim stopped." << endl;
        return false;
    }
    return true;
}

string createTimeString() {
    auto t = time(nullptr);
    auto tm = *localtime(&t);
    ostringstream oss;
    oss << put_time(&tm, "%d-%m-%Y_%H-%M-%S");
    return oss.str();
}

string createFileOutputString(const string &prefix, const string &time) {
    ostringstream oss;
    oss << "./" << time << "#" << prefix << "#CACHESIZE_" << CACHE_SIZE << "-BLOCKSIZE_" << CACHE_BLOCK_SIZE
        << "-DIRECT_" << DIRECT_CACHE << "-WAYS_" << CACHE_WAYS << ".sim";
    return oss.str();
}

/* Function prints and writes to file:
 * - simple hit/miss count
 * - approximation of fetching time due to hits and misses */
void PrintSimpleLog(MemCache *cache, vector<size_t> &addr_trace, const string &prefix,
                    const string &time, bool file_write_enabled) {

    double L1_hit_fetch_time = cache->cache_hits_ * cache->hit_latency_;
    double L1_miss_data_fetch_time_cache_hierarchy = cache->cache_misses_ * cache->aat_L1_miss;
    double L1_miss_data_fetch_time_mem = cache->cache_misses_ * MEM_LATENCY;

    cout << "/------- cache config --------/" << endl
         << "Cache size: " << cache->size_ / 1024 << " kb" << endl
         << "Block size: " << cache->block_size_ << " byte" << endl
         << "Block offset bits: " << cache->kbits_ << endl
         << "Set/Line index bits: " << cache->nbits_ << endl
         << "L1 hit-latency: " << cache->hit_latency_ << " ns" << endl
         << "L2 hit-latency: " << L2_HIT_LATENCY << " ns" << endl
         << "L2 miss-rate: " << L2_MISSRATE * 100 << " %" << endl;

    if (L3_HIT_LATENCY != 0)
        cout << "L3 hit-latency: " << L3_HIT_LATENCY << " ns" << endl
             << "L3 miss-rate: " << L3_MISSRATE * 100 << " %" << endl;

    if (!DIRECT_CACHE) {
        cout << "Cache mapping: associative" << endl
             << "Sets: " << cache->sets_ << endl
             << "Ways: " << cache->ways_ << endl
             << "Set Lines: " << cache->cache_set_ptr_->set_lines_ << endl << endl;
    } else {
        cout << "Cache mapping: direct" << endl
             << "Lines: " << cache->lines_ << endl << endl;
    }

    cout << "/------- hit / miss evaluation --------/" << endl
         << "/ addr_trace size: " << addr_trace.size() << endl
         << "/ L1 hits: " << cache->cache_hits_ << endl
         << "/ L1 misses: " << cache->cache_misses_ << endl
         << "/ L1 miss-rate: " << (cache->cache_misses_ / ((cache->cache_hits_+cache->cache_misses_) / (double)100)) << "%" << endl
         << "/ L1 replacements: " << cache->replacements_ << endl << endl

         << "/------- approximated data fetching time consumption --------/" << endl
         << "/ L1 miss average access time (3-Level cache hierarchy): " << cache->aat_L1_miss << " ns" << endl
         << "/ L1 hit complete data fetch time: " << L1_hit_fetch_time / (double) 1000 << " µs" << endl
         << "/ L1 miss complete data fetch time (3-Level cache hierarchy): "
         << L1_miss_data_fetch_time_cache_hierarchy / (double) 1000 << " µs" << endl
         << "/ complete data fetch time (3-Level cache hierarchy): "
         << (L1_miss_data_fetch_time_cache_hierarchy + L1_hit_fetch_time) / (double) 1000 << " µs" << endl
         << "/ complete data fetch time (L1 cache + main memory): "
         << (L1_hit_fetch_time + L1_miss_data_fetch_time_mem) / (double) 1000 << " µs" << endl
         << "/ complete data fetch time (main memory): "
         << ((cache->cache_hits_ + cache->cache_misses_) * MEM_LATENCY) / (double) 1000 << " µs" << endl << endl;

    if (file_write_enabled) {
        string fn = createFileOutputString(prefix, time);
        ofstream ofs(fn, ostream::out);
        if (!ofs.is_open()) {
            cout << "Failed to open file. " << fn << endl;
            goto out;
        }

        ofs << "/------- cache config --------/" << endl
            << "Cache size: " << cache->size_ / 1024 << " kb" << endl
            << "Block size: " << cache->block_size_ << " byte" << endl
            << "Block offset bits: " << cache->kbits_ << endl
            << "Set/Line index bits: " << cache->nbits_ << endl
            << "L1 hit-latency: " << cache->hit_latency_ << " ns" << endl
            << "L2 hit-latency: " << L2_HIT_LATENCY << " ns" << endl
            << "L2 miss-rate: " << L2_MISSRATE * 100 << " %" << endl;

        if (L3_HIT_LATENCY != 0)
            ofs << "L3 hit-latency: " << L3_HIT_LATENCY << " ns" << endl
                << "L3 miss-rate: " << L3_MISSRATE * 100 << " %" << endl;

        if (!DIRECT_CACHE) {
            ofs << "Cache mapping: associative" << endl
                << "Sets: " << cache->sets_ << endl
                << "Ways: " << cache->ways_ << endl
                << "Set Lines: " << cache->cache_set_ptr_->set_lines_ << endl << endl;
        } else {
            ofs << "Cache mapping: direct" << endl
                << "Lines: " << cache->lines_ << endl << endl;
        }

        ofs << "/------- hit / miss evaluation --------/" << endl
            << "/ addr_trace size: " << addr_trace.size() << endl
            << "/ L1 hits: " << cache->cache_hits_ << endl
            << "/ L1 misses: " << cache->cache_misses_ << endl
            << "/ L1 replacements: " << cache->replacements_ << endl << endl

            << "/------- approximated data fetching time consumption --------/" << endl
            << "/ L1 hit complete data fetch time: " << L1_hit_fetch_time / (double) 1000 << " µs" << endl
            << "/ L1 miss average access time (3-Level cache hierarchy): " << cache->aat_L1_miss << endl
            << "/ L1 miss complete data fetch time (3-Level cache hierarchy): "
            << L1_miss_data_fetch_time_cache_hierarchy / (double) 1000 << " µs" << endl
            << "/ complete data fetch time (3-Level cache hierarchy): "
            << (L1_miss_data_fetch_time_cache_hierarchy + L1_hit_fetch_time) / (double) 1000 << " µs" << endl
            << "/ complete data fetch time (L1 cache + main memory): "
            << (L1_hit_fetch_time + L1_miss_data_fetch_time_mem) / (double) 1000 << " µs" << endl
            << "/ complete data fetch time (main memory): "
            << ((cache->cache_hits_ + cache->cache_misses_) * MEM_LATENCY) / (double) 1000 << " µs" << endl << endl;
        ofs.close();
    }
    out:
    return;
}

/* Function records access times of L1 + Main memory */
void
RecordAccessTimeL1Memory(MemCache *cache, vector<double> &access_t, int64_t &last_hit_count, int64_t &last_miss_count) {
    access_t.push_back((double) (cache->cache_hits_ - last_hit_count) * cache->hit_latency_ +
                       (double) (cache->cache_misses_ - last_miss_count) * MEM_LATENCY);
}

/* Function records access times of L1 + L2 + L3 + Main memory */
void
RecordAccessTimeCacheHierachy(MemCache *cache, vector<double> &access_t, int64_t &last_hit_count,
                              int64_t &last_miss_count) {
    access_t.push_back((double) (cache->cache_hits_ - last_hit_count) * cache->hit_latency_ +
                       (double) (cache->cache_misses_ - last_miss_count) * cache->aat_L1_miss);
}

/* Function writes mean values to a file for evaluation*/
void WriteAccessTimeToFile(vector<double> &mean, const string &prefix, const string &time) {

    string fn = createFileOutputString(prefix, time);
    ofstream ofs(fn, ostream::out);
    if (!ofs.is_open()) {
        cout << "Failed to open file. " << fn << endl;
        goto out;
    }
    copy(mean.rbegin(), mean.rend(), ostream_iterator<double>(ofs, "\n"));
    ofs.close();
    out:
    return;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        cout << "Wrong program invocation." << endl
             << "Missing tracefile." << endl
             << "Usage: " << argv[0] << " /path/to/tracefile" << " trace_name" << endl
             << "CacheSim stopped." << endl;
        return 0;
    }

    vector<size_t> addr_trace;
    unsigned size = 0;                          // size = future work
    if (ReadFromFile(addr_trace, argv[1])) {    // read trace file
        // create cache
        MemCache__create();

        vector<double> access_t_L1_mem;
        vector<double> access_t_cache_hierarchy;
        int64_t last_hit_count = 0;
        int64_t last_miss_count = 0;

        // check for hit miss
        for (auto it = addr_trace.begin(); it != addr_trace.end(); ++it) {
            check_hit_miss(*it, size);
            RecordAccessTimeL1Memory(getCacheInstance(), access_t_L1_mem, last_hit_count, last_miss_count);
            RecordAccessTimeCacheHierachy(getCacheInstance(), access_t_cache_hierarchy, last_hit_count,
                                          last_miss_count);
            last_hit_count = getCacheInstance()->cache_hits_;
            last_miss_count = getCacheInstance()->cache_misses_;
        }

        // print some stuff
        string tracename(argv[2]);
        string time = createTimeString();
        PrintSimpleLog(getCacheInstance(), addr_trace, "simple#"+tracename, time, true);
        WriteAccessTimeToFile(access_t_L1_mem, "access_t_L1_mem#"+tracename, time);
        WriteAccessTimeToFile(access_t_cache_hierarchy, "access_t_cache_hierarchy#"+tracename, time);

        // free cache
        MemCache__free();

    } else {
        return 0;
    }
    return 1;
}
