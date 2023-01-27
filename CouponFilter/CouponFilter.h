#ifndef _COUPONFILTER_H
#define _COUPONFILTER_H

#include "iostream"
#include "ctime"
#include "cmath"
#include "cstring"
#include "set"

#include "../lib/MurmurHash3.h"
#include "../lib/Xorshift.h"
#include "../alg/UpdateInterface.h"

using namespace std;

#define MAX_VALUE 0xFFFFFFFF
#define NUM_BITS 8      // size of each item


typedef struct
{
    uint32_t m;
    uint32_t c;
    double p;
    char mea_tag;
} Cpf_Param;


// @params: m - num of filter units
//          c - size of each filter unit (in bits)
//          prefix_p - prefix value for the probability of drawing a coupon, p = prefix_p * 0.5 ** 32
//          mea_tag - f - (frequency-based); c - (cardinality-based); p - (persistence-based)
class CouponFilter
{
private:
    double p;
    uint32_t m;
    uint32_t c;
    char mea_tag;

    uint32_t seed;       // seed for murmurhash
    uint8_t * bitmap;
    UpdateInterface * update_interface;
    xorshift::xorshift32 rng;

    uint32_t get_unit_index(uint64_t flow_id)
    {
        uint64_t hash_val[2];
        char hash_input_str[9] = {0};
        memcpy(hash_input_str, &flow_id, sizeof(uint64_t));
        MurmurHash3_x86_128(hash_input_str, 8, seed, hash_val);
        return hash_val[0] % m;
    };

    int get_coupon_index(uint64_t flow_id, uint64_t ele_id)
    {
        uint32_t hash_val = 0;
        if(mea_tag == 'f') {
            char hash_input_str[9] = {0};
            uint32_t rand_val = rng();
            memcpy(hash_input_str, &flow_id, sizeof(uint32_t));
//            memcpy(hash_input_str+4, &ele_id, sizeof(uint32_t));
            memcpy(hash_input_str+4, &rand_val, sizeof(uint32_t));
            MurmurHash3_x86_32(hash_input_str, 8, seed, &hash_val);
        }
        else if(mea_tag == 'c') {
            char hash_input_str[9] = {0};
            memcpy(hash_input_str, &flow_id, sizeof(uint32_t));
            memcpy(hash_input_str+4, &ele_id, sizeof(uint32_t));

            MurmurHash3_x86_32(hash_input_str, 8, seed, &hash_val);
        }
        else if(mea_tag == 'p') {
            char hash_input_str[13] = {0};
            memcpy(hash_input_str, &flow_id, sizeof(uint64_t));
            memcpy(hash_input_str+8, &ele_id, sizeof(uint32_t));

            MurmurHash3_x86_32(hash_input_str, 12, seed, &hash_val);
        }

        for(int i=0; i<c; i++) {
            if(double(hash_val) < p * (i+1) * uint32_t(MAX_VALUE)) {
                return i;
            }
        }
        return -1;
    };

public:
    CouponFilter(uint32_t _m, uint32_t _c, double _p, char _mea_tag)
    {
        m = _m;
        c = _c;
        p = _p;
        mea_tag = _mea_tag;

        srand(time(NULL));
        seed = uint32_t(rand());
        rng.seed(uint32_t(rand()));

        bitmap = new uint8_t[m];
        memset(bitmap, 0, sizeof(uint8_t)*m);
    };

    ~CouponFilter()
    {
        delete [] bitmap;
    };

    void init_update_interface(UpdateInterface * interface)
    {
        update_interface = interface;
    };

    void update(uint64_t flow_id, uint64_t ele_id)
    {
        int coupon_index = get_coupon_index(flow_id, ele_id);
        uint32_t unit_index = get_unit_index(flow_id);
        if(coupon_index >= 0) {
//            cout << coupon_index << endl;
            bitmap[unit_index]  = bitmap[unit_index] | (1 << coupon_index);
        }
        if(bitmap[unit_index] == (uint8_t(1<<c) - 1)) {
            update_interface->update(flow_id, ele_id);
        }
    };

    bool query(uint64_t flow_id, uint64_t ele_id)
    {
        uint32_t unit_index = get_unit_index(flow_id);

        if(bitmap[unit_index] == (uint8_t(1<<c) - 1)) {
            return true;
        }
        return false;
    };

};

#endif //_COUPONFILTER_H
