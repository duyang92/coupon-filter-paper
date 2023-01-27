#include <iostream>
#include "iomanip"
#include "sstream"
#include "cstdio"
#include <cmath>
#include "ctime"
#include "map"
#include "vector"

#include "lib/CAIDA_Util.h"
#include "lib/CAIDA_Anls.h"

#include "alg/CountMin.h"
#include "CouponFilter/CouponFilter.h"

using namespace std;


const char mea_tag = 'f';
const bool is_unique = false;
const uint32_t num_slice = 1;
const string mea_method = "cm";
const uint32_t rounds = 15;

uint32_t total_mem_KB = 300;
uint32_t total_memory;
uint32_t mea_th = 100;
//string eval_target = "raw_cm";
string eval_target = "var_filter_th";

uint32_t cm_num_hash_func = 4;
uint32_t cm_counter_size = 20;

uint32_t tar_filter_th = 65;
uint32_t cpf_m = 276824;
uint32_t cpf_c = 2;
double cpf_p = 0.016000 ;
double filter_th = 61.126129;


void raw_cm(vector<Pkt_Info> dataset, string res_path, uint32_t round_index) {
    class Freq_Anls<num_slice> anls_instance;
    anls_instance.load();

    uint32_t cm_arr_width = total_memory / cm_counter_size / cm_num_hash_func;
    CountMin_onchip cm_ins(cm_arr_width, cm_num_hash_func);

    uint32_t dataset_size = dataset.size();
    Pkt_Info *p = &dataset[0];
    for(uint32_t item_index = 0; item_index < dataset_size; item_index++) {
        cm_ins.update(p[item_index].source_addr, p[item_index].target_addr);
    }

    vector<double> are;
    string save_path = res_path + to_string(round_index) + ".txt";
    ofstream out_res(save_path, ios::binary);
    map<uint32_t , uint32_t> res = anls_instance.query_all();
    map<uint32_t, uint32_t>::iterator itr = res.begin();
    for(; itr != res.end(); itr++) {
        if(itr->second > mea_th) {
            double ans_t = cm_ins.query(itr->first);
            out_res << itr->second << " ";
            out_res << ans_t << " ";

            double diff = double(itr->second) - ans_t;
            if(diff < 0) diff = -diff;
            diff = diff / itr->second;
            are.push_back(diff);
        }
    }
    out_res.close();

    double sum = 0;
    for(int i = 0; i < are.size(); i++) {
        sum += are[i];
    }
    cout << res_path << "   ";
    cout << "raw_cm: " << sum / are.size() << endl;
}

void cpf_with_cm(vector<Pkt_Info> dataset, string res_path, uint32_t round_index) {
    uint32_t cm_arr_width = (total_memory - cpf_m * cpf_c) / cm_counter_size / cm_num_hash_func;
    CountMin_onchip cm_ins(cm_arr_width, cm_num_hash_func);

    CouponFilter cpf_ins(cpf_m, cpf_c, cpf_p, mea_tag);
    cpf_ins.init_update_interface(&cm_ins);

    uint32_t dataset_size = dataset.size();
    Pkt_Info *p = &dataset[0];
    for(uint32_t item_index = 0; item_index < dataset_size; item_index++) {
        cpf_ins.update(p[item_index].source_addr, p[item_index].target_addr);
    }

    vector<double> are;
    string save_path = res_path + to_string(round_index) + ".txt";
    ofstream out_res(save_path, ios::binary);
    class Freq_Anls<num_slice> anls_instance;
    anls_instance.load();
    map<uint32_t , uint32_t> res = anls_instance.query_all();
    map<uint32_t, uint32_t>::iterator itr = res.begin();
    for(; itr != res.end(); itr++) {
        if(itr->second > mea_th) {
            double ans_t = cm_ins.query(itr->first);
            if(ans_t != 0) {
                ans_t += filter_th;
            }
            else {
                ans_t = 1;
            }

            out_res << itr->second << " ";
            out_res << ans_t << " ";

            double diff = double(itr->second) - ans_t;
            if(diff < 0) diff = -diff;
            are.push_back(diff / double(itr->second));
        }
    }
    out_res.close();

    double sum = 0;
    for(int i = 0; i < are.size(); i++) {
        sum += are[i];
    }
    cout << res_path << "   ";
    cout << "cpf_cm: " << sum / are.size() << endl;

}


int main(int argc, char** argv) {

    for (int i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], "-total_mem") == 0)
        {
            string str_temp = string(argv[i + 1]);
            total_mem_KB = strtoul(str_temp.c_str(), NULL, 10);
        }
        else if(strcmp(argv[i], "-mea_th") == 0)
        {
            string str_temp = string(argv[i + 1]);
            mea_th = strtoul(str_temp.c_str(), NULL, 10);
        }
        else if (strcmp(argv[i], "-eval_target") == 0)
        {
            string str_temp = string(argv[i + 1]);
            eval_target = str_temp;
        }
        else if (strcmp(argv[i], "-tar_filter_th") == 0)
        {
            string str_temp = string(argv[i + 1]);
            tar_filter_th = strtod(str_temp.c_str(), NULL);
        }
        else if(strcmp(argv[i], "-cpf_m") == 0)
        {
            string str_temp = string(argv[i + 1]);
            cpf_m = strtoul(str_temp.c_str(), NULL, 10);
        }
        else if(strcmp(argv[i], "-cpf_c") == 0)
        {
            string str_temp = string(argv[i + 1]);
            cpf_c = strtoul(str_temp.c_str(), NULL, 10);
        }
        else if (strcmp(argv[i], "-cpf_p") == 0)
        {
            string str_temp = string(argv[i + 1]);
            cpf_p = strtod(str_temp.c_str(), NULL);
        }
        else if (strcmp(argv[i], "-filter_th") == 0)
        {
            string str_temp = string(argv[i + 1]);
            filter_th = strtod(str_temp.c_str(), NULL);
        }
        else if (strcmp(argv[i], "-cm_num_hash_func") == 0)
        {
            string str_temp = string(argv[i + 1]);
            cm_num_hash_func = strtoul(str_temp.c_str(), NULL, 10);
        }
        else if (strcmp(argv[i], "-cm_counter_size") == 0)
        {
            string str_temp = string(argv[i + 1]);
            cm_counter_size = strtoul(str_temp.c_str(), NULL, 10);
        }
    }

    total_memory = uint32_t(double(total_mem_KB) / 1000 * 1024 * 1024 * 8);

    CAIDA_Util<is_unique> caida_util;
    caida_util.read_caida_file_uint32(num_slice);
    vector<Pkt_Info> dataset = caida_util.get_caida_dataset();

    if (eval_target.compare("raw_cm") == 0) {
        string res_path = "../res/cpf_" + mea_method + "_" + to_string(total_mem_KB) + "KB/";
        res_path += "raw_cm/" + to_string(mea_th) + "/";

        for (int round_index = 1; round_index <= rounds; round_index++) {
            raw_cm(dataset, res_path, round_index);
        }
    }
    else {
        string res_path = "../res/cpf_" + mea_method + "_" + to_string(total_mem_KB) + "KB/";
        if(eval_target.compare("var_filter_th") == 0) {
            res_path += "filter_th/"+ to_string(mea_th) + "/" + to_string(tar_filter_th) + "/";
            cout << res_path << endl;
        }
        else if(eval_target.compare("var_mem_percent") == 0) {
            stringstream ss;
            ss << setiosflags(ios::fixed) << setprecision(2) << double(cpf_m * cpf_c) / total_memory;
            res_path += "mem_percent/"+ to_string(mea_th) + "/" + ss.str() + "/";
            cout << res_path << endl;
        }

        for(int round_index = 1; round_index <= rounds; round_index++) {
            cpf_with_cm(dataset, res_path, round_index);
        }
    }

}



