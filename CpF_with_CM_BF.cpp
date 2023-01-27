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

#include "alg/CM_BF.h"
#include "CouponFilter/CouponFilter.h"

using namespace std;

const char mea_tag = 'p';
const bool is_unique = false;
const uint32_t num_slice = 1;
const string mea_method = "cmbf";
const uint32_t rounds = 15;
const uint32_t num_window = 1600;
uint32_t len_window;

uint32_t total_mem_KB = 300;
uint32_t total_memory;
uint32_t mea_th = 320;
string eval_target = "raw_cmbf";
//string eval_target = "var_mem_percent";

uint32_t tar_filter_th = 35;
uint32_t cpf_m = 125829;
uint32_t cpf_c = 2;
double cpf_p = 0.028000;
double filter_th = 35.231598;
uint32_t cmbf_num_hash = 2;


void raw_cmbf(vector<Pkt_Info> dataset, string res_path, uint32_t round_index) {
    CM_BF cmbf_ins(cmbf_num_hash, total_memory / cmbf_num_hash / (8 + sizeof(int32_t) * 8));

    int32_t number = 0, window_id = 0;
    uint32_t dataset_size = dataset.size();
    Pkt_Info *p = &dataset[0];
    for(uint32_t item_index = 0; item_index < dataset_size; item_index++) {
        if(window_id > num_window) break;
        if(number % len_window == 0) {
            window_id++;
            cmbf_ins.new_window(window_id);
        }

        uint64_t item_id = p[item_index].source_addr;
        item_id = item_id << 32;
        item_id += p[item_index].target_addr;
        cmbf_ins.update(item_id, window_id);
        number++;
    }

    string save_path = res_path + to_string(round_index) + ".txt";
    ofstream out_res(save_path, ios::binary);
    class Perst_Anls<num_slice> anls_instance;
    anls_instance.load();
    vector<double> aae;
    map<uint64_t , uint32_t> res = anls_instance.query_all();
    map<uint64_t, uint32_t>::iterator itr = res.begin();

    for(; itr != res.end(); itr++) {
        if(itr->second > mea_th) {
            uint32_t ans_t = cmbf_ins.query(itr->first);
            int diff = int(itr->second) - ans_t;
            if(diff < 0) diff = -diff;
            aae.push_back(diff/itr->second);

            out_res << int(itr->second) << " ";
            out_res << ans_t << " ";
        }
    }

    out_res.close();

    double sum = 0;
    for(int i = 0; i < aae.size(); i++) {
        sum += aae[i];
    }
    cout << "raw: " << sum / aae.size() << endl;
}


void cpf_with_cmbf(vector<Pkt_Info> dataset, string res_path, uint32_t round_index) {
    CM_BF cmbf_ins(cmbf_num_hash, (total_memory - cpf_m * cpf_c) / cmbf_num_hash / (8 + sizeof(int32_t) * 8));
    CouponFilter cpf_ins(cpf_m, cpf_c, cpf_p, mea_tag);
    cpf_ins.init_update_interface(&cmbf_ins);

    int32_t number = 0, window_id = 0;
    uint32_t dataset_size = dataset.size();
    Pkt_Info *p = &dataset[0];
    for(uint32_t item_index = 0; item_index < dataset_size; item_index++) {
        if(window_id > num_window) break;
        if(number % len_window == 0) {
            window_id++;
            cmbf_ins.new_window(window_id);
        }

        uint64_t item_id = p[item_index].source_addr;
        item_id = item_id << 32;
        item_id += p[item_index].target_addr;
        cpf_ins.update(item_id, window_id);

        number++;
    }

    vector<double> aae;
    string save_path = res_path + to_string(round_index) + ".txt";
    ofstream out_res(save_path, ios::binary);
    class Perst_Anls<num_slice> anls_instance;
    anls_instance.load();
    map<uint64_t , uint32_t> res = anls_instance.query_all();
    map<uint64_t, uint32_t>::iterator itr = res.begin();
    for(; itr != res.end(); itr++) {
        if(itr->second > mea_th) {
            uint32_t ans_t = cmbf_ins.query(itr->first);
            ans_t += filter_th;
            int diff = int(itr->second) - ans_t;
            if(diff < 0) diff = -diff;
            aae.push_back(diff/itr->second);

            out_res << int(itr->second) << " ";
            out_res << ans_t << " ";
        }
    }
    out_res.close();

    double sum = 0;
    for(int i = 0; i < aae.size(); i++) {
        sum += aae[i];
    }
    cout << "cpf+cmbf: " << sum / aae.size() << endl;
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
        else if (strcmp(argv[i], "-cmbf_num_hash") == 0)
        {
            string str_temp = string(argv[i + 1]);
            cmbf_num_hash = strtoul(str_temp.c_str(), NULL, 10);
        }
    }

    total_memory = uint32_t(double(total_mem_KB) / 1000 * 1024 * 1024 * 8);

    CAIDA_Util<is_unique> caida_util;
    caida_util.read_caida_file_uint32(num_slice);
    vector<Pkt_Info> dataset = caida_util.get_caida_dataset();
    len_window = dataset.size() / num_window;

    if (eval_target.compare("raw_cmbf") == 0) {
        string res_path = "../res/cpf_" + mea_method + "_" + to_string(total_mem_KB) + "KB/";
        res_path += "raw_cmbf/" + to_string(mea_th) + "/";

        for (int round_index = 1; round_index <= rounds; round_index++) {
            raw_cmbf(dataset, res_path, round_index);
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
            cpf_with_cmbf(dataset, res_path, round_index);
        }
    }
}


