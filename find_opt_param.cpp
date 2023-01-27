#include "string"
#include "sys/stat.h"
#include "fstream"
#include "map"

#include "lib/CAIDA_Util.h"
#include "lib/CAIDA_Anls.h"
#include "CouponFilter/CouponFilter.h"
#include "CouponFilter/CouponFilter_PrAnls.h"

using namespace std;

#define E 2.718281828459
#define NUM_V_REG 128
#define CM_NUM_HASH 4
#define CM_COUNTER_SIZE 20
#define ONOFF_NUM_HASH 2
#define CMBF_NUM_HASH 2


typedef struct {
    Cpf_Param param;
    PrAnls_Res res;
} PrAnls_Case;

PrAnls_Res get_anls_res(const Cpf_Param ins_param, vector<double> ele_distr_perflow, uint32_t num_flow)
{
    CouponFilter_PrAnls cpf_anls(ins_param.m, ins_param.c, ins_param.p, ins_param.mea_tag);

    vector<double> pr_num_flow_perunit = cpf_anls.pr_num_flow_perunit(num_flow);
    double ** pr_ele_of_gatheredflow = cpf_anls.pr_ele_of_gatheredflow(pr_num_flow_perunit.size()-1, ele_distr_perflow);
    vector<double> pr_ele_per_unit = cpf_anls.pr_ele_perunit(pr_num_flow_perunit, pr_ele_of_gatheredflow);
    vector<vector<double>> ans = cpf_anls.pr_coupon_of_ele();
    PrAnls_Res result = cpf_anls.get_biases(ans, pr_ele_per_unit);

    return result;
}

void run_case(const char mea_tag, const uint32_t total_mem_KB) {
    const auto total_memory = uint32_t(double(total_mem_KB) / 1000 * 1024 * 1024 * 8);

    vector<double> ele_distr_perflow;
    uint32_t num_flow;
    string file_path;
    if(mea_tag == 'f') {
        class Freq_Anls<NUM_SLICE_FREQ> anls_instance;
        anls_instance.load();
        ele_distr_perflow = anls_instance.get_perflow_distr();
        num_flow = anls_instance.get_num_flow();
        file_path = "../res/" + to_string(total_mem_KB) + "KB_freq.anls";
    }
    else if(mea_tag == 'c'){
        class Cardi_Anls<NUM_SLICE_CARDI> anls_instance;
        anls_instance.load();
        ele_distr_perflow = anls_instance.get_perflow_distr();
        num_flow = anls_instance.get_num_flow();
        file_path = "../res/" + to_string(total_mem_KB) + "KB_cardi.anls";
    }
    else if(mea_tag == 'p') {
        class Perst_Anls<NUM_SLICE_PERST> anls_instance;
        anls_instance.load();
        ele_distr_perflow = anls_instance.get_perflow_distr();
        num_flow = anls_instance.get_num_flow();
        file_path = "../res/" + to_string(total_mem_KB) + "KB_perst.anls";
    }

    ofstream out_param(file_path);
    vector<double> pr_coup_vec;
    for(double pr_coup = 0.001; pr_coup <= 0.3; pr_coup+=0.001) {
        pr_coup_vec.push_back(pr_coup);
    }
    for(double pr_coup = 0.31; pr_coup <= 0.5; pr_coup+=0.01) {
        pr_coup_vec.push_back(pr_coup);
    }

    for(uint32_t i = 0; i < pr_coup_vec.size(); i++) {
        double pr_coup = pr_coup_vec[i];
        for(uint32_t size_unit = 2; size_unit <= 4; size_unit++) {
            if(pr_coup * size_unit > 1){
                continue;
            }
            for(double mem_prop = 0.04; mem_prop <= 0.5; mem_prop+=0.02) {
                Cpf_Param param;
                param.c = size_unit;
                param.m = uint32_t(total_memory * mem_prop) / param.c;
                param.p = pr_coup;
                param.mea_tag = mea_tag;

                PrAnls_Res res = get_anls_res(param, ele_distr_perflow, num_flow);

                string save_str;
                save_str.clear();
                save_str += (to_string(param.m) + "," + to_string(param.c) + "," + to_string(param.p) + ":");
                save_str += (to_string(res.avg_consumed_ele) + "," + to_string(res.var_consumed_ele) + "," + to_string(res.num_passed_ele) + "\n");
                out_param << save_str;
                cout << save_str;
            }
        }
    }
    out_param.close();
}

vector<PrAnls_Case> load(string file_path, const char mea_tag){
    vector<PrAnls_Case> ans;
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

        for(int i = 0; i < data_vec.size(); i++) {
            if(data_vec[i].size() <= 1) continue;

            vector<string> line_vec;
            str_split(data_vec[i], line_vec, ':');
            vector<string> param_vec;
            str_split(line_vec[0], param_vec, ',');
            vector<string> res_vec;
            str_split(line_vec[1], res_vec, ',');

            PrAnls_Case temp;
            temp.param.m = stoul(param_vec[0]);
            temp.param.c = stoul(param_vec[1]);
            temp.param.p = stod(param_vec[2]);
            temp.param.mea_tag = mea_tag;
            temp.res.avg_consumed_ele = stod(res_vec[0]);
            temp.res.var_consumed_ele = stod(res_vec[1]);
            temp.res.num_passed_ele = stod(res_vec[2]);
            ans.push_back(temp);
        }
    }
    return ans;
}

vector<PrAnls_Case> get_case_around_th(const vector<PrAnls_Case> &case_vec, double th) {
    vector<PrAnls_Case> ans;
    ans.clear();
    double det_range = 0;
    while(ans.size() <= 30) {
        det_range += 0.2;
        ans.clear();
        for(int i = 0; i < case_vec.size(); i++) {
            double achieved_th = case_vec[i].res.avg_consumed_ele;
            if(achieved_th >= th - det_range && achieved_th <= th + det_range && case_vec[i].param.c == 4 && case_vec[i].param.m <= 16777) {
                ans.push_back(case_vec[i]);
            }
        }
    }
    return ans;
}

vector<PrAnls_Case> get_case_around_percent(const vector<PrAnls_Case> &case_vec, double mem_percent, uint32_t mea_th, uint32_t total_memory) {
    vector<PrAnls_Case> ans;
    ans.clear();

    for(int i = 0; i < case_vec.size(); i++) {
        double real_percent = double(case_vec[i].param.m * case_vec[i].param.c) / total_memory;
        stringstream ss;
        ss << setiosflags(ios::fixed) << setprecision(2) << real_percent;
        real_percent = stod(ss.str());

        if(abs(mem_percent - real_percent) < 0.00001 && case_vec[i].param.c == 4 && case_vec[i].res.avg_consumed_ele >= 55) {
            double achieved_th = case_vec[i].res.avg_consumed_ele;
            if(achieved_th < mea_th) {
                ans.push_back(case_vec[i]);
            }
        }
    }
    return ans;
}

PrAnls_Case get_param(const vector<PrAnls_Case> &case_around_th, double mea_th, uint32_t total_memory, const string mea_method) {
    auto min_var = double (0xFFFFFFFFFFFF);
    PrAnls_Case min_var_case;
    for(int i = 0; i < case_around_th.size(); i++) {
        PrAnls_Case case_itr = case_around_th[i];
        uint32_t sketch_memory = total_memory - case_itr.param.m * case_itr.param.c;
        double var_case = 0;

        if(case_itr.param.mea_tag == 'f') {
            if(mea_method.compare("cm") == 0) {
                // ========== Variance Cal for Count-Min Sketch ==========
                auto l = uint32_t(double(sketch_memory) / CM_NUM_HASH / CM_COUNTER_SIZE);
                var_case = pow((case_itr.res.num_passed_ele * E / l) / 7, 2) * (1 - 1 / pow(E, CM_NUM_HASH));
            }
        }
        else if(case_itr.param.mea_tag == 'c') {
            if(mea_method.compare("nds") == 0) {
                // ========== Variance Cal for NDS Method ==========
                double p_NDS = exp(- case_itr.res.num_passed_ele / double(sketch_memory));
                var_case = mea_th * (1 - p_NDS) /  p_NDS;
            }
            else if(mea_method.compare("vhll") == 0) {
                // ========== Variance Cal for vHLL Method ==========
                uint32_t size_phy_reg = uint32_t(ceil(log2(32-log2(double(NUM_V_REG)))));
                uint32_t m = sketch_memory / size_phy_reg;
                var_case = pow(double(case_itr.res.num_passed_ele / (m / NUM_V_REG)) / 30, 2) * (1 - 1 / pow(E, NUM_V_REG));
            }
        }
        else if(case_itr.param.mea_tag == 'p') {
            if(mea_method.compare("onoff") == 0) {
                // ========== Variance Cal for On-Off Sketch ==========
                auto l = uint32_t(double(sketch_memory) / ONOFF_NUM_HASH / (1.0 + 32.0));
                var_case = pow((case_itr.res.num_passed_ele * E / l) / 13, 2) * (1 - 1 / pow(E, ONOFF_NUM_HASH));
            }
            else if(mea_method.compare("cmbf") == 0) {
                // ========== Variance Cal for On-Off Sketch ==========
                auto l = uint32_t(double(sketch_memory) / ONOFF_NUM_HASH / (1.0 + 32.0));
                var_case = pow((case_itr.res.num_passed_ele * E / l) / 5, 2) * (1 - 1 / pow(E, CMBF_NUM_HASH));
            }
        }

        var_case += case_itr.res.var_consumed_ele;
        if(var_case < min_var) {
            min_var = var_case;
            min_var_case = case_itr;
        }
    }
//    cout << "Var: " << min_var << "   " ;
    return min_var_case;
}


int main() {

    const char mea_tag = 'c';
    const string mea_method = "nds";
    const uint32_t total_mem_KB = 100;
    const auto total_memory = uint32_t(double(total_mem_KB) / 1000 * 1024 * 1024 * 8);
    cout << "Total Mem: " << total_mem_KB << " KB" << endl;
//    string var_type = "var_filter_th";
    string var_type = "var_mem_percent";
    uint32_t mea_th = 200;

    string file_path;
    if(mea_tag == 'f')      file_path = "../res/" + to_string(total_mem_KB) + "KB_freq.anls";
    else if(mea_tag == 'c')     file_path = "../res/" + to_string(total_mem_KB) + "KB_cardi.anls";
    else if(mea_tag == 'p')     file_path = "../res/" + to_string(total_mem_KB) + "KB_perst.anls";

//  //  //  run_case(mea_tag, total_mem_KB);

    if(var_type.compare("var_filter_th") == 0) {
        vector<PrAnls_Case> case_vec = load(file_path, mea_tag);
        for(uint32_t th = 5; th <= 100; th += 5) {
            vector<PrAnls_Case> case_around_th = get_case_around_th(case_vec, th);
            PrAnls_Case opt_case = get_param(case_around_th, mea_th, total_memory, mea_method);

            string save_str;
            save_str.clear();
            save_str += "  \"param=(" + to_string(int(th)) + " ";
            save_str += to_string(opt_case.param.m) + " ";
            save_str += to_string(opt_case.param.c) + " ";
            save_str += to_string(opt_case.param.p) + " ";
            save_str += to_string(opt_case.res.avg_consumed_ele) + " ";

            if(mea_method.compare("nds") == 0) {
                double p_NDS = exp(- opt_case.res.num_passed_ele / double(total_memory - opt_case.param.m * opt_case.param.c));
                save_str += to_string(p_NDS) + ")\"\n";
            }
            else if(mea_method.compare("vhll") == 0) {
                save_str += to_string(NUM_V_REG) + ")\"\n";
            }
            else if(mea_method.compare("cm") == 0) {
                save_str += to_string(CM_NUM_HASH) + " " + to_string(CM_COUNTER_SIZE) + ")\"\n";
            }
            else if(mea_method.compare("onoff") == 0) {
                save_str += to_string(ONOFF_NUM_HASH) + ")\"\n";
            }
            else if(mea_method.compare("cmbf") == 0) {
                save_str += to_string(CMBF_NUM_HASH) + ")\"\n";
            }
            else {
                save_str += ")\"\n";
            }

            cout << save_str;
        }
    }
    else if(var_type.compare("var_mem_percent") == 0) {
        vector<PrAnls_Case> case_vec = load(file_path, mea_tag);
        for(double mem_percent = 0.04; mem_percent <= 0.51; ) {
            vector<PrAnls_Case> case_around_percent = get_case_around_percent(case_vec, mem_percent, mea_th, total_memory);
            PrAnls_Case opt_case = get_param(case_around_percent, mea_th, total_memory, mea_method);

            stringstream ss;
            ss << setiosflags(ios::fixed) << setprecision(2) << mem_percent;
            cout << "  \"param=(" << ss.str() << " ";
            string save_str;
            save_str.clear();
            save_str += to_string(opt_case.param.m) + " ";
            save_str += to_string(opt_case.param.c) + " ";
            save_str += to_string(opt_case.param.p) + " ";
            save_str += to_string(opt_case.res.avg_consumed_ele) + " ";

            if(mea_method.compare("nds") == 0) {
                double p_NDS = exp(- opt_case.res.num_passed_ele / double(total_memory - opt_case.param.m * opt_case.param.c));
                save_str += to_string(p_NDS) + ")\"\n";
            }
            else if(mea_method.compare("vhll") == 0) {
                save_str += to_string(NUM_V_REG) + ")\"\n";
            }
            else if(mea_method.compare("cm") == 0) {
                save_str += to_string(CM_NUM_HASH) + " " + to_string(CM_COUNTER_SIZE) + ")\"\n";
            }
            else if(mea_method.compare("onoff") == 0) {
                save_str += to_string(ONOFF_NUM_HASH) + ")\"\n";
            }
            else if(mea_method.compare("cmbf") == 0) {
                save_str += to_string(CMBF_NUM_HASH) + ")\"\n";
            }
            else {
                save_str += "\n";
            }

            cout << save_str;
            mem_percent += 0.02;
        }
    }

}

