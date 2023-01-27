#ifndef _VHLL_H
#define _VHLL_H

#include "iostream"
#include "ctime"
#include "cmath"
#include "set"
#include "map"
#include "vector"
#include "cstring"

#include "./UpdateInterface.h"
#include "../lib/MurmurHash3.h"

using namespace std;


class vHLL: public UpdateInterface
{
private:
    uint32_t m;
    uint32_t s;
    uint32_t * seeds;
    uint32_t num_leading_bit;
    uint32_t hash_seed;
    uint32_t * register_arr;

    double cardi_all_flow;
    double alpha;

    set<uint64_t> flows;

public:
    vHLL(const uint32_t num_phy_register, const uint32_t num_virtual_register) {
        m = num_phy_register;
        s = num_virtual_register;

        srand(time(NULL));
        seeds = new uint32_t[num_virtual_register];
        set<uint32_t> seed_set;
        while(seed_set.size() < num_virtual_register) {
            seed_set.insert(uint32_t(rand()));
        }
        set<uint32_t>::iterator itr = seed_set.begin();
        uint32_t index = 0;
        for(; itr != seed_set.end(); itr++) {
            seeds[index] = *itr;
            index++;
        }

        num_leading_bit = floor(log10(double(num_virtual_register))/log10(2.0));
        hash_seed = uint32_t(rand());
        register_arr = new uint32_t[num_phy_register];
        memset(register_arr, 0, sizeof(uint32_t) * num_phy_register);

        cardi_all_flow = 0;
        if(num_virtual_register == 16)       alpha = 0.673;
        else if(num_virtual_register == 32)      alpha = 0.697;
        else if(num_virtual_register == 64)      alpha = 0.709;
        else    alpha = (0.7213 / (1 + (1.079 / num_virtual_register)));
    }

    ~vHLL() {
        delete [] seeds;
    }

    void update(uint64_t flow_id, uint64_t ele_id) {
        flows.insert(flow_id);
        uint32_t ele_hash_val = 0;

        char hash_input_str[5] = {0};
        memcpy(hash_input_str, &ele_id, sizeof(uint32_t));
        MurmurHash3_x86_32(hash_input_str, 4, hash_seed, &ele_hash_val);
        uint32_t p_part = ele_hash_val >> (sizeof(uint32_t) * 8 - num_leading_bit);
        uint32_t q_part = ele_hash_val - (p_part << (sizeof(uint32_t) * 8 - num_leading_bit));

        uint32_t left_most = 0;
        while(q_part) {
            left_most += 1;
            q_part = q_part >> 1;
        }
        left_most = sizeof(uint32_t) * 8 - num_leading_bit - left_most + 1;

        uint32_t xor_val = uint32_t(flow_id) ^ seeds[p_part];
        memcpy(hash_input_str, &xor_val, sizeof(uint32_t));
        uint32_t index_phy_register = 0;
        MurmurHash3_x86_32(hash_input_str, 4, hash_seed, &index_phy_register);
        index_phy_register = index_phy_register % m;
        register_arr[index_phy_register] = max(register_arr[index_phy_register], left_most);
    }

    void update_param() {
        double zero_ratio = 0;
        double sum_register_arr = 0;
        for(int i = 0; i < m; i++) {
            sum_register_arr += pow(2.0, -double(register_arr[i]));
            if(register_arr[i] == 0)    zero_ratio += 1;
        }
        zero_ratio = zero_ratio / m;
        double temp_cardi_all_flow = (0.7213 / (1 + (1.079 / m))) * pow(double(m), 2) / sum_register_arr;

        if(temp_cardi_all_flow <= 2.5 * m) {
            if(zero_ratio != 0) {
                cardi_all_flow = - double(m) * log(zero_ratio);
            }
        }
        else if(temp_cardi_all_flow > pow(2.0, 32) / 30) {
            cardi_all_flow = - pow(2.0, 32) * log(1 - temp_cardi_all_flow / pow(2.0, 32));
        }
        else if(temp_cardi_all_flow < pow(2.0, 32) / 30) {
            cardi_all_flow = temp_cardi_all_flow;
        }
    }

    double query(uint64_t flow_id) {
        if(flows.count(flow_id) != 0) {
            double zero_ratio_v_reg = 0;
            double sum_v_reg = 0;
            for(int i = 0; i < s; i++) {
                uint32_t seed = seeds[i];

                char hash_input_str[5] = {0};
                uint32_t xor_val = uint32_t(flow_id) ^ seed;
                memcpy(hash_input_str, &xor_val, sizeof(uint32_t));
                uint32_t index_phy_register = 0;
                MurmurHash3_x86_32(hash_input_str, 4, hash_seed, &index_phy_register);
                index_phy_register = index_phy_register % m;
                sum_v_reg += pow(2.0, -double(register_arr[index_phy_register]));
                if(register_arr[index_phy_register] == 0)    zero_ratio_v_reg += 1;
            }
            zero_ratio_v_reg = zero_ratio_v_reg / s;
            double flow_cardi = alpha * pow(s, 2) / sum_v_reg;

            if(flow_cardi <= 2.5 * s) {
                if(zero_ratio_v_reg != 0) {
                    flow_cardi = - log(zero_ratio_v_reg) * s - cardi_all_flow * s / m;
                }
                else {
                    flow_cardi = flow_cardi - cardi_all_flow * s / m;
                }
            }
            else if(flow_cardi > pow(2.0, 32) / 30) {
                flow_cardi = - pow(2.0, 32) * log(1 - flow_cardi / pow(2.0, 32)) - cardi_all_flow * s / m;
            }
            else if(flow_cardi < pow(2.0, 32) / 30) {
                flow_cardi = flow_cardi - cardi_all_flow * s / m;
            }

            return flow_cardi;
        }
        else {
            return 0;
        }

    }

    map<uint64_t, uint32_t> spread_esti(vector<uint64_t> &flow_vec) {
        map<uint64_t, uint32_t> ans;
        update_param();
        vector<uint64_t>::iterator itr = flow_vec.begin();
        for(; itr != flow_vec.end(); itr++) {
            ans[*itr] = query(*itr);
        }
        return ans;
    }

};



#endif // _VHLL_H
