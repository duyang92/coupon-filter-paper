#ifndef _COUNTMIN_H
#define _COUNTMIN_H

#include "iostream"
#include "ctime"
#include "vector"
#include "map"

#include "./UpdateInterface.h"
#include "../lib/MurmurHash3.h"

using namespace std;

# define MIN(a,b)  (a < b ? a : b)


class CountMin_onchip: public UpdateInterface
{
private:
    unsigned int w, d;
    uint32_t  * seeds;
    uint32_t ** flow_counter;

public:
    CountMin_onchip(const uint32_t arr_width, const uint32_t num_hash_func) {
        w = arr_width;
        d = num_hash_func;
        seeds = new uint32_t[num_hash_func];
        flow_counter = new uint32_t * [num_hash_func];

        srand(time(NULL));
        for(int i = 0; i < num_hash_func; i++) {
            flow_counter[i] = new uint32_t [arr_width];
            memset(flow_counter[i], 0, sizeof(uint32_t) * arr_width);
            seeds[i] = uint32_t(rand());
        }
    }

    ~CountMin_onchip() {
        delete [] flow_counter;
        delete [] seeds;
    }

    void update(uint64_t flow_id, uint64_t ele_id) {
        uint32_t flow_id_t = uint32_t(flow_id);
        char hash_input_str[5] = {0};
        memcpy(hash_input_str, &flow_id_t, sizeof(uint32_t));
        for(int i = 0; i < d; i++) {
            uint32_t hash_val = 0;
            MurmurHash3_x86_32(hash_input_str, 4, seeds[i], &hash_val);
            flow_counter[i][hash_val % w] += 1;
        }
    }

    uint32_t query(uint64_t flow_id) {
        uint32_t min_val = 0xFFFFFFFF;
        char hash_input_str[5] = {0};
        memcpy(hash_input_str, &flow_id, sizeof(uint32_t));

        for(int i = 0; i < d; i++) {
            uint32_t hash_val = 0;
            MurmurHash3_x86_32(hash_input_str, 4, seeds[i], &hash_val);
            min_val = MIN(min_val, flow_counter[i][hash_val % w]);
        }
        return min_val;
    }

    map<uint64_t, uint32_t> freq_esti(vector<uint64_t> &flow_vec) {
        map<uint64_t, uint32_t> ans;
        vector<uint64_t>::iterator itr = flow_vec.begin();
        for(; itr != flow_vec.end(); itr++) {
            ans[*itr] = query(*itr);
        }
        return ans;
    }

};


#endif // _COUNTMIN_H
