#ifndef _CAIDA_ANLS_H
#define _CAIDA_ANLS_H

#include "iostream"
#include "vector"
#include "map"
#include "set"
#include "sys/stat.h"
#include "fstream"
#include "cstring"

#include "./Basic_Util.h"

using namespace std;


template<uint32_t num_slice>
class Freq_Anls
{
private:
    uint32_t num_flow;
    vector<double> dist_perflow;
    uint64_t num_total_ele;
    map<uint32_t, uint32_t> flow_counter;

    static bool cmp_val(const pair<uint32_t, uint32_t> &a, const pair<uint32_t, uint32_t> &b) {
        return a.second > b.second;
    }

public:
    Freq_Anls() {
        num_flow = 0;
        dist_perflow.clear();
        num_total_ele = 0;
        flow_counter.clear();
    }

    ~Freq_Anls() {
        dist_perflow.clear();
        flow_counter.clear();
    }

    void update(uint32_t flow_id, uint32_t ele_id) {
        num_total_ele += 1;
        if(flow_counter.count(flow_id) == 0) {
            num_flow += 1;
            flow_counter[flow_id] = 1;
        }
        else {
            flow_counter[flow_id] += 1;
        }

    }

    uint32_t query(uint32_t flow_id) {
        if(flow_counter.count(flow_id) == 0) {
            return 0;
        }
        return flow_counter[flow_id];
    }

    map<uint32_t, uint32_t> query_all() {
        return flow_counter;
    }

    uint32_t get_num_flow() {
        return num_flow;
    }

    uint64_t get_num_ele() {
        return num_total_ele;
    }

    vector<double> get_perflow_distr() {
        map<uint32_t, double, less<uint32_t>> freq_count;
        uint32_t max_val = 0;
        map<uint32_t, uint32_t>::iterator itr = flow_counter.begin();
        for(; itr != flow_counter.end(); itr++) {
            if(max_val < itr->second) {
                max_val = itr->second;
            }

            if(freq_count.count(itr->second) == 0) {
                freq_count[itr->second] = 1;
            }
            else {
                freq_count[itr->second] += 1;
            }
        }

        double distr[max_val+1];
        map<uint32_t, double>::iterator itrr = freq_count.begin();
        map<uint32_t, double>::iterator itrr_next = freq_count.begin();
        itrr_next++;
        memset(distr, 0, sizeof(distr));
        for(; itrr != freq_count.end(); itrr++) {
            if(itrr_next == freq_count.end() || itrr_next->first - itrr->first == 1) {
                distr[itrr->first] = itrr->second / num_flow;
            }
            else {
                double pr_temp = itrr->second / (itrr_next->first - itrr->first) / num_flow;
                for(uint32_t index = itrr->first; index < itrr_next->first; index++) {
                    distr[index] = pr_temp;
                }
            }
            if(itrr_next != freq_count.end())   itrr_next++;
        }
        vector<double> ans(distr, distr+max_val);
        return ans;
    }

    vector<uint32_t> query_top_k(uint32_t k_val) {
        vector<pair<uint32_t, uint32_t>> vec;
        for(map<uint32_t, uint32_t>::iterator itr = flow_counter.begin(); itr != flow_counter.end(); itr++) {
            vec.push_back(pair<uint32_t, uint32_t>(itr->first, itr->second));
        }
        sort(vec.begin(), vec.end(), cmp_val);
        uint32_t min_val = vec[k_val-1].second;
        vector<uint32_t> ans;
        for(int index = 0; index < vec.size(); index++) {
            if (vec[index].second >= min_val) {
                ans.push_back(vec[index].first);
            }
            else {
                break;
            }
        }
        return ans;
    }

    void save() {
        string save_str = "";
        save_str += to_string(num_flow) + "\n";
        save_str += to_string(num_total_ele) + "\n";
        map<uint32_t, uint32_t>::iterator itr = flow_counter.begin();
        for(; itr != flow_counter.end(); itr++) {
            save_str += to_string(itr->first) + "," + to_string(itr->second) + "\n";
        }
        string file_path = "../res/" + to_string(num_slice) + "mins_anls.freq";
        ofstream out_param(file_path);
        out_param << save_str;
        out_param.close();
    }

    void load() {
        string file_path = "../res/" + to_string(num_slice) + "mins_anls.freq";
        struct stat buffer;
        if(stat(file_path.c_str(), &buffer) != 0) {
            cout << "File does not exist!" << endl;
            exit(-1);
        }
        else {
            ifstream in_caida(file_path, ios::binary);
            vector<char> buf(in_caida.seekg(0, ios::end).tellg());
            in_caida.seekg(0, ios::beg).read(&buf[0], static_cast<streamsize>(buf.size()));
            string str_data(buf.begin(), buf.end());
            buf.clear();

            vector<string> data_vec;
            str_split(str_data, data_vec, '\n');
            str_data.clear();

            num_flow = stoul(data_vec[0]);
            num_total_ele = stoull(data_vec[1]);
            for(int i = 2; i < data_vec.size(); i++) {
                if(data_vec[i].size() <= 1) continue;

                vector<string> line_vec;
                str_split(data_vec[i], line_vec, ',');
                flow_counter[stoul(line_vec[0])] = stoul(line_vec[1]);
            }
        }
    }

};


template<uint32_t num_slice>
class Cardi_Anls
{
private:
    uint32_t num_flow;
    vector<double> dist_perflow;
    uint64_t num_total_ele;
    map<uint32_t, set<uint32_t>> flow_collector;
    map<uint32_t, uint32_t> flow_counter;

public:
    Cardi_Anls() {
        num_flow = 0;
        dist_perflow.clear();
        num_total_ele = 0;
        flow_collector.clear();
    }

    ~Cardi_Anls() {
        dist_perflow.clear();
        flow_collector.clear();
    }

    void update(uint32_t flow_id, uint32_t ele_id) {
        if(flow_collector.count(flow_id) == 0) {
            num_flow += 1;
        }
        flow_collector[flow_id].insert(ele_id);
        flow_counter[flow_id] = flow_collector[flow_id].size();
    }

    uint32_t query(uint32_t flow_id) {
        if(flow_collector.count(flow_id) == 0) {
            return 0;
        }
        return flow_collector[flow_id].size();
    }

    map<uint32_t, uint32_t> query_all() {
        return flow_counter;
    }

    uint32_t get_num_flow() {
        return num_flow;
    }

    uint64_t get_num_ele() {
        num_total_ele = 0;
        map<uint32_t, uint32_t>::iterator itr = flow_counter.begin();
        for(; itr != flow_counter.end(); itr++) {
            num_total_ele += itr->second;
        }
        return num_total_ele;
    }

    vector<double> get_perflow_distr() {
        map<uint32_t, double, less<uint32_t>> cardi_count;
        uint32_t max_val = 0;
        map<uint32_t, uint32_t>::iterator itr = flow_counter.begin();
        for(; itr != flow_counter.end(); itr++) {
            if(max_val < itr->second) {
                max_val = itr->second;
            }

            if(cardi_count.count(itr->second) == 0) {
                cardi_count[itr->second] = 1;
            }
            else {
                cardi_count[itr->second] += 1;
            }
        }

        double distr[max_val+1];
        map<uint32_t, double>::iterator itrr = cardi_count.begin();
        map<uint32_t, double>::iterator itrr_next = cardi_count.begin();
        itrr_next++;
        memset(distr, 0, sizeof(distr));
        for(; itrr != cardi_count.end(); itrr++) {
            if(itrr_next == cardi_count.end() || itrr_next->first - itrr->first == 1) {
                distr[itrr->first] = itrr->second / num_flow;
            }
            else {
                double pr_temp = itrr->second / (itrr_next->first - itrr->first) / num_flow;
                for(uint32_t index = itrr->first; index < itrr_next->first; index++) {
                    distr[index] = pr_temp;
                }
            }
            if(itrr_next != cardi_count.end())   itrr_next++;
        }
        vector<double> ans(distr, distr+max_val);
        return ans;
    }

    void save() {
        string save_str = "";
        save_str += to_string(num_flow) + "\n";
        save_str += to_string(get_num_ele()) + "\n";
        map<uint32_t, set<uint32_t>>::iterator itr = flow_collector.begin();
        for(; itr != flow_collector.end(); itr++) {
            save_str += to_string(itr->first) + "," + to_string(itr->second.size()) + "\n";
        }
        string file_path = "../res/" + to_string(num_slice) + "mins_anls.cardi";
        ofstream out_param(file_path);
        out_param << save_str;
        out_param.close();
    }

    void load() {
        string file_path = "../res/" + to_string(num_slice) + "mins_anls.cardi";
        struct stat buffer;
        if(stat(file_path.c_str(), &buffer) != 0) {
            cout << "Cardi_Anls: File does not exist!" << endl;
            exit(-1);
        }
        else {
            ifstream in_caida(file_path, ios::binary);
            vector<char> buf(in_caida.seekg(0, ios::end).tellg());
            in_caida.seekg(0, ios::beg).read(&buf[0], static_cast<streamsize>(buf.size()));
            string str_data(buf.begin(), buf.end());
            buf.clear();

            vector<string> data_vec;
            str_split(str_data, data_vec, '\n');
            str_data.clear();

            num_flow = stoul(data_vec[0]);
            num_total_ele = stoull(data_vec[1]);
            for(int i = 2; i < data_vec.size(); i++) {
                if(data_vec[i].size() <= 1) continue;

                vector<string> line_vec;
                str_split(data_vec[i], line_vec, ',');
                flow_counter[stoul(line_vec[0])] = stoul(line_vec[1]);
            }
        }
    }

};

template<uint32_t num_slice>
class Perst_Anls
{
private:
    uint32_t num_flow;
    map<uint64_t, set<uint32_t>> flow_collector;
    map<uint64_t, uint32_t> flow_countor;

public:
    Perst_Anls() {
        num_flow = 0;
        flow_collector.clear();
        flow_countor.clear();
    }

    ~Perst_Anls() {
        flow_collector.clear();
        flow_countor.clear();
    }

    void update(uint64_t item_id, uint32_t window_id) {
        if(flow_collector.count(item_id) == 0) {
            num_flow += 1;
        }
        flow_collector[item_id].insert(window_id);
        flow_countor[item_id] = flow_collector[item_id].size();
    }

    uint32_t query(uint64_t item_id) {
        if(flow_countor.count(item_id) == 0) {
            return 0;
        }
        return flow_countor[item_id];
    }

    map<uint64_t, uint32_t> query_all() {
        return flow_countor;
    }

    uint32_t  get_num_flow() {
        return num_flow;
    }

    vector<double> get_perflow_distr() {
        map<uint32_t, double, less<uint32_t>> cardi_count;
        uint32_t max_val = 0;
        map<uint64_t, uint32_t>::iterator itr = flow_countor.begin();
        for(; itr != flow_countor.end(); itr++) {
            if(max_val < itr->second) {
                max_val = itr->second;
            }

            if(cardi_count.count(itr->second) == 0) {
                cardi_count[itr->second] = 1;
            }
            else {
                cardi_count[itr->second] += 1;
            }
        }

        double distr[max_val+1];
        map<uint32_t, double>::iterator itrr = cardi_count.begin();
        map<uint32_t, double>::iterator itrr_next = cardi_count.begin();
        itrr_next++;
        memset(distr, 0, sizeof(distr));
        for(; itrr != cardi_count.end(); itrr++) {
            if(itrr_next == cardi_count.end() || itrr_next->first - itrr->first == 1) {
                distr[itrr->first] = itrr->second / num_flow;
            }
            else {
                double pr_temp = itrr->second / (itrr_next->first - itrr->first) / num_flow;
                for(uint32_t index = itrr->first; index < itrr_next->first; index++) {
                    distr[index] = pr_temp;
                }
            }
            if(itrr_next != cardi_count.end())   itrr_next++;
        }
        vector<double> ans(distr, distr+max_val);
        return ans;
    }

    void save() {
        string save_str = "";
        save_str += to_string(num_flow) + "\n";
        map<uint64_t, uint32_t>::iterator itr = flow_countor.begin();
        for(; itr != flow_countor.end(); itr++) {
            save_str += to_string(itr->first) + "," + to_string(itr->second) + "\n";
        }
        string file_path = "../res/" + to_string(num_slice) + "mins_anls.perst";
        ofstream out_param(file_path);
        out_param << save_str;
        out_param.close();
    }

    void load() {
        string file_path = "../res/" + to_string(num_slice) + "mins_anls.perst";
        struct stat buffer;
        if(stat(file_path.c_str(), &buffer) != 0) {
            cout << "File does not exist!" << endl;
            exit(-1);
        }
        else {
            ifstream in_caida(file_path, ios::binary);
            vector<char> buf(in_caida.seekg(0, ios::end).tellg());
            in_caida.seekg(0, ios::beg).read(&buf[0], static_cast<streamsize>(buf.size()));
            string str_data(buf.begin(), buf.end());
            buf.clear();

            vector<string> data_vec;
            str_split(str_data, data_vec, '\n');
            str_data.clear();

            num_flow = stoul(data_vec[0]);
            for(int i = 1; i < data_vec.size(); i++) {
                if(data_vec[i].size() <= 1) continue;

                vector<string> line_vec;
                str_split(data_vec[i], line_vec, ',');
                flow_countor[stoull(line_vec[0])] = stoul(line_vec[1]);
            }
        }
    }
};



#endif // _CAIDA_ANLS_H
