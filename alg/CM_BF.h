#ifndef _CM_BF_H
#define _CM_BF_H

#include "limits.h"
#include "string"

#include "./UpdateInterface.h"
#include "../lib/bitset.h"
#include "../lib/MurmurHash3.h"

using namespace std;

#define BFSEED 101
#define MAX(a, b) (a > b? a:b)
#define MIN(a, b) (a < b? a:b)


class CM_BF: public UpdateInterface
{
public:
    struct BF{
        BF(uint32_t _hash_num, uint32_t _length):
            hash_num(_hash_num), length(_length){
                bf = new BitSet(length);
            }

        ~BF(){
            delete bf;
        }

        bool find(const uint64_t flow_id){
            for(uint32_t i = 0; i < hash_num; ++i){
                uint32_t pos = hash(flow_id, 1<<i + BFSEED) % length;
                if(!bf->Get(pos))
                    return false;
            }
            return true;
        }

        void insert(const uint64_t flow_id){
            for(uint32_t i = 0; i < hash_num; ++i){
                uint32_t pos = hash(flow_id, 1<<i + BFSEED) % length;
                bf->Set(pos);
            }
        }

        void clear(){
            bf->Clear();
        }

        inline uint32_t hash(uint64_t data, uint32_t seed = 0){
            char hash_input_str[9] = {0};
            memcpy(hash_input_str, &data, sizeof(uint64_t));
            uint32_t hash_val;
            MurmurHash3_x86_32(hash_input_str, 8, seed, &hash_val);
            return hash_val;
        }

        BitSet* bf;

        const uint32_t hash_num;
        const uint32_t length;
    };

    CM_BF(uint32_t _hash_num, uint32_t _length):
        hash_num(_hash_num), length(_length){
            bf = new BF(_hash_num * 2, _length * _hash_num * 8);
            counters = new uint32_t * [hash_num];
            for(uint32_t i = 0;i < hash_num;++i){
                counters[i] = new uint32_t [length];
                memset(counters[i], 0, length * sizeof(uint32_t));
            }
        }

    ~CM_BF(){
        for(uint32_t i = 0; i < hash_num; ++i){
            delete [] counters[i];
        }
        delete bf;
        delete [] counters;
    }

    void update(const uint64_t flow_id, const uint64_t ele_id){
        if(!bf->find(flow_id)){
            bf->insert(flow_id);
            for(uint32_t i = 0; i < hash_num; ++i){
                uint32_t pos = hash(flow_id, 1<<i) % length;
                counters[i][pos] += 1;
            }
        }
    }

    uint32_t query(const uint64_t item){
        uint32_t ret = INT_MAX;
        for(uint32_t i = 0; i < hash_num; ++i){
            uint32_t pos = hash(item, 1<<i) % length;
            ret = MIN(ret, counters[i][pos]);
        }
        return ret;
    }

    void new_window(const uint32_t window){
        bf->clear();
    }

    inline uint32_t hash(uint64_t data, uint32_t seed = 0){
        char hash_input_str[9] = {0};
        memcpy(hash_input_str, &data, sizeof(uint64_t));
        uint32_t hash_val;
        MurmurHash3_x86_32(hash_input_str, 8, seed, &hash_val);
        return hash_val;
    }

private:
    BF* bf;

    const uint32_t hash_num;
    const uint32_t length;

    uint32_t ** counters;
};

#endif // _CM_BF_H
