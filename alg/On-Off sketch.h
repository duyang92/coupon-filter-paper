#ifndef _ON_OFF_SKETCH_H
#define _ON_OFF_SKETCH_H

#include "limits.h"

#include "./UpdateInterface.h"
#include "../lib/bitset.h"
#include "../lib/MurmurHash3.h"

using namespace std;

#define MAX(a, b) (a > b? a:b)
#define MIN(a, b) (a < b? a:b)


class OnOff_sketch: public UpdateInterface
{
public:
    OnOff_sketch(uint32_t _hash_num, uint32_t _length):
            hash_num(_hash_num), length(_length){
        counters = new uint32_t * [hash_num];
        bitsets = new BitSet* [hash_num];
        for(uint32_t i = 0; i < hash_num; ++i){
            counters[i] = new uint32_t [length];
            bitsets[i] = new BitSet(length);
            memset(counters[i], 0, length * sizeof(uint32_t));
        }
    }

    ~OnOff_sketch(){
        for(uint32_t i = 0; i < hash_num; ++i){
            delete [] counters[i];
            delete bitsets[i];
        }
        delete [] counters;
        delete [] bitsets;
    }

    void update(const uint64_t flow_id, const uint64_t ele_id){
        for(uint32_t i = 0; i < hash_num; ++i){
            uint32_t pos = hash(flow_id, i) % length;
            counters[i][pos] += (!bitsets[i]->SetNGet(pos));
        }
    }

    uint32_t query(const uint64_t flow_id){
        uint32_t ret = INT_MAX;
        for(uint32_t i = 0; i < hash_num; ++i){
            uint32_t pos = hash(flow_id, i) % length;
            ret = MIN(ret, counters[i][pos]);
        }
        return ret;
    }

    void new_window(const uint32_t window){
        for(uint32_t i = 0; i < hash_num; ++i){
            bitsets[i]->Clear();
        }
    }

    inline uint32_t hash(uint64_t data, uint32_t seed = 0){
        char hash_input_str[9] = {0};
        memcpy(hash_input_str, &data, sizeof(uint64_t));
        uint32_t hash_val;
        MurmurHash3_x86_32(hash_input_str, 8, seed, &hash_val);
        return hash_val;
    }

private:
    const uint32_t hash_num;
    const uint32_t length;

    BitSet** bitsets;
    uint32_t** counters;
};



#endif // _ON_OFF_SKETCH_H
