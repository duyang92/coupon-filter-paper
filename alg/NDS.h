#ifndef _NDS_H
#define _NDS_H

#include "iostream"
#include "cmath"
#include "ctime"
#include "map"
#include "cstring"

#include "./UpdateInterface.h"
#include "./QueryInterface.h"
#include "../lib/MurmurHash3.h"

using namespace std;

#define MAX_VALUE 0xFFFFFFFF
#define NUM_BITS 8      // size of each item


class NDS_Onchip: public UpdateInterface
{
private:
    uint32_t m;
    double p;
    uint8_t * bitmap;
    uint32_t sampling_seed;
    uint32_t filtering_seed;
    uint32_t num_set_bits;
    QueryInterface * query_interface;

    bool packet_is_sampled(uint64_t flow_id, uint64_t ele_id) {
        uint32_t hash_val = 0;

        char hash_input_str[9] = {0};
        memcpy(hash_input_str, &flow_id, sizeof(uint32_t));
        memcpy(hash_input_str+4, &ele_id, sizeof(uint32_t));

        MurmurHash3_x86_32(hash_input_str, 8, sampling_seed, &hash_val);
        if(hash_val < p / (1 - double(num_set_bits) / m) * uint32_t(MAX_VALUE)) {
            return true;
        }
        return false;
    };

public:
    NDS_Onchip(uint32_t num_bit, double sampling_pr) {
        m = num_bit;
        p = sampling_pr;

        srand(time(NULL));
        sampling_seed = uint32_t(rand());
        filtering_seed = uint32_t(rand());

        uint32_t len_bitmap = ceil(float(m)/float(NUM_BITS));
        bitmap = new uint8_t[len_bitmap];
        memset(bitmap, 0, sizeof(uint8_t)*len_bitmap);
        num_set_bits = 0;

        query_interface = NULL;
    };

    ~NDS_Onchip() {
        delete [] bitmap;
    };

    void init_query_interface(QueryInterface * interface) {
        query_interface = interface;
    }

    void update(uint64_t flow_id, uint64_t ele_id) {
        uint32_t hash_val = 0;
        char hash_input_str[9] = {0};
        memcpy(hash_input_str, &flow_id, sizeof(uint32_t));
        memcpy(hash_input_str+4, &ele_id, sizeof(uint32_t));

        MurmurHash3_x86_32(hash_input_str, 8, filtering_seed, &hash_val);
        uint32_t bitarray_index = (hash_val % m) / NUM_BITS;
        uint32_t bit_index_in_bitarray = (hash_val % m) % NUM_BITS;

        uint8_t temp = uint8_t(pow(2, NUM_BITS-1-bit_index_in_bitarray));
        if( uint8_t(bitmap[bitarray_index] & temp) == 0 ) {
            bitmap[bitarray_index] = bitmap[bitarray_index] | temp;
            num_set_bits += 1;
            if(query_interface != NULL && packet_is_sampled(flow_id, ele_id)) {
                query_interface->to_offchip(flow_id, ele_id);
            }
        }
    };
};


class NDS_Offchip: public QueryInterface
{
private:
    map<uint64_t, uint32_t> offchip_counter;
    double p;

public:
    NDS_Offchip(double _p) {
        p = _p;
        offchip_counter.clear();
    }

    void to_offchip(uint64_t flow_id, uint64_t ele_id) {
        if(offchip_counter.count(flow_id) == 0) {
            offchip_counter[flow_id] = 1;
        }
        else {
            offchip_counter[flow_id] = offchip_counter.at(flow_id) + 1;
        }
    }

    uint32_t query(uint64_t flow_id) {
        if(offchip_counter.count(flow_id) == 0) {
            return 1;
        }
        else {
            return round(offchip_counter[flow_id] / p);
        }
    };

    map<uint64_t, uint32_t> spread_esti() {
        map<uint64_t , uint32_t> results(offchip_counter);
        map<uint64_t, uint32_t>::iterator iter;
        for(iter = results.begin(); iter != results.end(); iter++) {
            iter->second = round(iter->second / p);
        }
        return results;
    };

};

#endif // _NDS_H
