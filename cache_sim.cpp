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
    oss << "./" << time << "_" << prefix << "-CACHESIZE_" << CACHE_SIZE << "-BLOCKSIZE_" << CACHE_BLOCK_SIZE
        << "-DIRECT_" << DIRECT_CACHE << "-WAYS_" << CACHE_WAYS << ".sim";
    return oss.str();
}

/* Function prints and writes to file:
 * - simple hit/miss count
 * - approximation of fetching time due to hits and misses */
void PrintSimpleLog(MemCache *cache, vector<size_t> &addr_trace, const string &prefix,
                    const string &time, bool file_write_enabled) {

    double data_fetching_time_w_cache = (double)cache->cache_hits_ * cache->hit_latency_ +
            (double)cache->cache_misses_ * cache->miss_latency_;

    double data_fetching_time_wo_cache = ((double)cache->cache_hits_ + (double)cache->cache_misses_) * cache->miss_latency_;

    cout << "/------- hit miss evaluation --------/" << endl
         << "/ addr_trace size: " << addr_trace.size() << endl
         << "/ hits: " << cache->cache_hits_ << endl
         << "/ misses: " << cache->cache_misses_ << endl
         << "/ replacements: " << cache->replacements_ << endl << endl
         << "/------- approximated data fetching time consumption --------/" << endl
         << "/ complete hit latency: " << cache->cache_hits_ * cache->hit_latency_ << " ns" << endl
         << "/ complete miss latency: " << cache->cache_misses_ * cache->miss_latency_ << " ns" << endl
         << "/ fetching time w cache: " << data_fetching_time_w_cache << " ns" << " / "
         << data_fetching_time_w_cache / (double)1000 << " µs" << endl
         << "/ fetching time w/o cache: "<< data_fetching_time_wo_cache << " ns" << " / "
         << data_fetching_time_wo_cache / (double)1000 << " µs" << endl << endl;

    if (file_write_enabled) {
        string fn = createFileOutputString(prefix, time);
        ofstream ofs(fn, ostream::out);
        if (!ofs.is_open()) {
            cout << "Failed to open file. " << fn << endl;
            goto out;
        }

        ofs << "/------- hit miss evaluation --------/" << endl
            << "/ addr_trace size: " << addr_trace.size() << endl
            << "/ hits: " << cache->cache_hits_ << endl
            << "/ misses: " << cache->cache_misses_ << endl
            << "/ replacements: " << cache->replacements_ << endl << endl
            << "/------- approximated data fetching time consumption --------/" << endl
            << "/ complete hit latency: " << cache->cache_hits_ * cache->hit_latency_ << " ns" << endl
            << "/ complete miss latency: " << cache->cache_misses_ * cache->miss_latency_ << " ns" << endl
            << "/ fetching time w cache: " << data_fetching_time_w_cache << " ns" << " / "
            << data_fetching_time_w_cache / (double)1000 << " µs" << endl
            << "/ fetching time w/o cache: "<< data_fetching_time_wo_cache << " ns" << " / "
            << data_fetching_time_wo_cache / (double)1000 << " µs" << endl << endl;
        ofs.close();
    }
out:
    return;
}

/* Function pushes value of mem access to vector */
void
RecordAccessTime(MemCache *cache, vector<double> &access_t, int64_t &last_hit_count, int64_t &last_miss_count) {
    access_t.push_back((double)(cache->cache_hits_ - last_hit_count) * cache->hit_latency_ +
                               (double)(cache->cache_misses_ - last_miss_count) * cache->miss_latency_);

    last_hit_count = cache->cache_hits_;
    last_miss_count = cache->cache_misses_;
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
    if (argc < 2) {
        cout << "Wrong program invocation." << endl
             << "Missing tracefile." << endl
             << "Usage: " << argv[0] << " /path/to/tracefile" << endl
             << "CacheSim stopped." << endl;
        return 0;
    }

    vector<size_t> addr_trace;
    unsigned size = 0;                          // size = future work
    if (ReadFromFile(addr_trace, argv[1])) {    // read trace file
        // create cache
        MemCache__create();

        vector<double> access_t;
        int64_t last_hit_count = 0;
        int64_t last_miss_count = 0;

        // check for hit miss
        for (auto it = addr_trace.begin(); it != addr_trace.end(); ++it) {
            check_hit_miss(*it, size);
            RecordAccessTime(getCacheInstance(), access_t, last_hit_count, last_miss_count);
        }

        // print some stuff
        string time = createTimeString();
        PrintSimpleLog(getCacheInstance(), addr_trace, "simple", time, true);
        WriteAccessTimeToFile(access_t, "access_t", time);

        // free cache
        MemCache__free();

    } else {
        return 0;
    }
    return 1;
}
