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

#include "alg/SpaceSaving.h"
#include "CouponFilter/CouponFilter.h"

using namespace std;


const char mea_tag = 'f';
const bool is_unique = false;
const uint32_t num_slice = 3;
const string mea_method = "ss";
const uint32_t rounds = 15;

uint32_t k_val = 1024;
string eval_target = "raw_ss";
//string eval_target = "var_filter_th";

uint32_t tar_filter_th = 65;
uint32_t cpf_m = 276824;
uint32_t cpf_c = 2;
double cpf_p = 0.016000 ;
double filter_th = 61.126129;


void raw_ss(vector<Pkt_Info> dataset, string res_path, uint32_t round_index) {
    class Freq_Anls<num_slice> anls_instance;
    anls_instance.load();

    SpaceSaving ss_ins(k_val * 5 / 2 + 1024);

    uint32_t dataset_size = dataset.size();
    Pkt_Info *p = &dataset[0];
    for(uint32_t item_index = 0; item_index < dataset_size; item_index++) {
        ss_ins.update(p[item_index].source_addr, p[item_index].target_addr);
    }

    double pre;
    string save_path = res_path + to_string(round_index) + ".txt";
    ofstream out_res(save_path, ios::binary);

    vector<uint32_t> res = anls_instance.query_top_k(k_val);
    vector<uint32_t> esti_res = ss_ins.get_top_k(k_val);

    vector<uint32_t>::iterator esti_itr = esti_res.begin();
    for(; esti_itr != esti_res.end(); esti_itr++) {
        out_res << *esti_itr << endl;
        vector<uint32_t>::iterator itr = res.begin();
        for(; itr != res.end(); itr++) {
            if(*itr == *esti_itr) {
                pre = pre + 1.0;
                break;
            }
        }
    }
    out_res.close();

    cout << res_path << "   ";
    cout << "raw_ss: " << pre / double(k_val) << endl;
}

void cpf_with_ss(vector<Pkt_Info> dataset, string res_path, uint32_t round_index) {
    SpaceSaving ss_ins(k_val * 5 / 2);
    CouponFilter cpf_ins(cpf_m, cpf_c, cpf_p, mea_tag);
    cpf_ins.init_update_interface(&ss_ins);

    uint32_t dataset_size = dataset.size();
    Pkt_Info *p = &dataset[0];
    for(uint32_t item_index = 0; item_index < dataset_size; item_index++) {
        cpf_ins.update(p[item_index].source_addr, p[item_index].target_addr);
    }

    double pre;
    string save_path = res_path + to_string(round_index) + ".txt";
    ofstream out_res(save_path, ios::binary);

    class Freq_Anls<num_slice> anls_instance;
    anls_instance.load();
    vector<uint32_t> res = anls_instance.query_top_k(k_val);
    vector<uint32_t> esti_res = ss_ins.get_top_k(k_val);

    vector<uint32_t>::iterator esti_itr = esti_res.begin();
    for(; esti_itr != esti_res.end(); esti_itr++) {
        out_res << *esti_itr << endl;
        vector<uint32_t>::iterator itr = res.begin();
        for(; itr != res.end(); itr++) {
            if(*itr == *esti_itr) {
                pre = pre + 1.0;
                break;
            }
        }
    }
    out_res.close();

    cout << res_path << "   ";
    cout << "cpf_cm: " << pre / double(k_val) << endl;

}


int main(int argc, char** argv) {

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-eval_target") == 0)
        {
            string str_temp = string(argv[i + 1]);
            eval_target = str_temp;
        }
        else if (strcmp(argv[i], "-k_val") == 0){
            string str_temp = string(argv[i + 1]);
            k_val = strtoul(str_temp.c_str(), NULL, 10);
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
    }

    CAIDA_Util<is_unique> caida_util;
    caida_util.read_caida_file_uint32(num_slice);
    vector<Pkt_Info> dataset = caida_util.get_caida_dataset();

    if (eval_target.compare("raw_cm") == 0) {
        string res_path = "../res/cpf_" + mea_method + "_top_" + to_string(k_val) + "/";
        res_path += "raw_ss/";

        for (int round_index = 1; round_index <= rounds; round_index++) {
            raw_ss(dataset, res_path, round_index);
        }
    }
    else {
        string res_path = "../res/cpf_" + mea_method + "_top_" + to_string(k_val) + "/";
        if(eval_target.compare("var_filter_th") == 0) {
            res_path += "filter_th/" + to_string(tar_filter_th) + "/";
            cout << res_path << endl;
        }

        for(int round_index = 1; round_index <= rounds; round_index++) {
            cpf_with_ss(dataset, res_path, round_index);
        }
    }

}



