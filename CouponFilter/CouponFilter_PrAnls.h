#ifndef _COUPONFILTER_PRANLS_H
#define _COUPONFILTER_PRANLS_H

#include "sys/stat.h"
#include "cstdint"
#include "algorithm"
#include "cmath"
#include "vector"
#include "fstream"
#include "iomanip"
#include "sstream"
#include "cstring"

#include "../lib/Basic_Util.h"

#define MAX_NUM_ELE 5000

using namespace std;


typedef struct
{
    double avg_consumed_ele;
    double var_consumed_ele;
    double num_passed_ele;
} PrAnls_Res;


uint64_t comb(uint32_t n, uint32_t m) {
    if(n == m || m == 0) return 1;

    uint64_t  ans = 1;
    m = min<uint64_t>(m, n-m);

    for(uint32_t i = 1; i <= m; i++) {
        ans = ans * (n-i+1) / i;
    }
    return ans;
}


double comb_with_pr(uint32_t n, uint32_t m, double pr) {
    double ans = 1;

    if(m == 0 || n == m) {
        if(m == 0) pr = 1 - pr;
        for(int i = 0; i < n / 10; i++) {
            ans = ans * pow(pr, 10);
        }
        ans = ans * pow(pr, n % 10);
        return ans;
    }

    if(n-m < m) {
        m = n - m;
        pr = 1 - pr;
    }
    uint32_t times = n - m;
    for(uint32_t i = 1; i <= m; i++) {
        ans = ans * (double(n-i+1) / i * pr);
        while(times > 0) {
            ans = ans * (1 - pr);
            times--;
            if(ans < 1) {
                break;
            }
        }
    }
    for(int i = 0; i < times / 10; i++) {
        ans = ans * pow((1 - pr), 10);
    }
    ans = ans * pow((1 - pr), times % 10);
    return ans;
}


class CouponFilter_PrAnls
{
private:
    uint32_t m;
    uint32_t c;
    double p;
    char mea_tag;
    double ** all_pr_coupon_of_ele;

public:
    CouponFilter_PrAnls(uint32_t num_unit, uint32_t num_coup, double pr_coup, char _mea_tag) {
        m = num_unit;
        c = num_coup;
        p = pr_coup;
        mea_tag = _mea_tag;
        all_pr_coupon_of_ele = new double * [num_coup+1];
    }

    ~CouponFilter_PrAnls(){
        delete [] all_pr_coupon_of_ele;
    }

    // @description: since more than one flows might be hashed in a certain unit,
    //               this function is to compute the probe of a unit that a certain number of flows are hashed to it.
    //               i.e. p(r)
    vector<double> pr_num_flow_perunit(uint32_t hist_num_flow) {
        vector<double> pr_vec;
        double sum = 0;
        int num_shared_flow = 0;

        while(sum <= 1 - pow(0.1, 10)) {
            double pr_temp = comb_with_pr(hist_num_flow, num_shared_flow, double(1.0 / m));
            if(isinf(pr_temp) || isnan(pr_temp)) {
                return pr_vec;
            }
            sum += pr_temp;
            pr_vec.push_back(pr_temp);
            num_shared_flow++;
        }
        return pr_vec;
    }

    // @description: this function is to compute the probe of the frequency/cardinality,
    //               when a certain number of flows are gathered together.   i.e. p(n|r)
    double ** pr_ele_of_gatheredflow(uint32_t th_num_gatheredflow, vector<double> dist_perflow) {
        const uint32_t max_num_gatheredflow = 200;
        auto ** all_pr = new double * [max_num_gatheredflow];
        string pr_file_path;
        if(mea_tag == 'c') pr_file_path = "../res/cardi_of_gatheredflows.pr";
        else if(mea_tag == 'f') pr_file_path = "../res/freq_of_gatheredflows.pr";
        else if(mea_tag == 'p') pr_file_path = "../res/perst_of_gatheredflows.pr";
        else exit(-1);

        struct stat buffer;
        if(stat(pr_file_path.c_str(), &buffer) == 0) {
            ifstream in_caida(pr_file_path, ios::binary);
            vector<char> buf(in_caida.seekg(0, ios::end).tellg());
            in_caida.seekg(0, ios::beg).read(&buf[0], static_cast<streamsize>(buf.size()));
            string str_data(buf.begin(), buf.end());
            buf.clear();
            
            vector<string> data_vec;
            str_split(str_data, data_vec, '\n');
            str_data.clear();
            for(int i = 0; i < data_vec.size(); i++) {
                if(data_vec[i].size() <= 1) continue;

                all_pr[i] = new double [MAX_NUM_ELE];
                memset(all_pr[i], 0, sizeof(double) * MAX_NUM_ELE);
                double * pr_ele_of_flows = all_pr[i];
                vector<string> line_vec;
                str_split(data_vec[i], line_vec, ',');
                for(int j = 0; j < line_vec.size(); j++) {
                    pr_ele_of_flows[j] = double(stold(line_vec[j]));
                }
            }
        }
        else {
            
            
            all_pr[0] = new double [MAX_NUM_ELE];
            memset(all_pr[0], 0, sizeof(double) * MAX_NUM_ELE);
            all_pr[0][0] = 1;
            all_pr[1] = new double [MAX_NUM_ELE];
            memset(all_pr[1], 0, sizeof(double) * MAX_NUM_ELE);
            if(MAX_NUM_ELE <= dist_perflow.size()) {
                memcpy(all_pr[1], &dist_perflow[0], MAX_NUM_ELE * sizeof(double));
            }
            else{
                memcpy(all_pr[1], &dist_perflow[0], dist_perflow.size() * sizeof(double));
            }

            for(uint32_t num_gatheredflow = 2; num_gatheredflow <= max_num_gatheredflow; num_gatheredflow++) {
                all_pr[num_gatheredflow] = new double [MAX_NUM_ELE];
                memset(all_pr[num_gatheredflow], 0, sizeof(double) * MAX_NUM_ELE);
                double * pr_ele_of_perflow = all_pr[1];
                double * prev_pr_ele_of_flows = all_pr[num_gatheredflow - 1];
                for(uint32_t num_ele = 0; num_ele < MAX_NUM_ELE; num_ele++) {
                    if(num_ele < num_gatheredflow) {
                        all_pr[num_gatheredflow][num_ele] = 0;
                    }
                    else {
                        double pr_temp = 0;
                        for(uint32_t j = 1; j < num_ele; j++) {
                            pr_temp += pr_ele_of_perflow[j] * prev_pr_ele_of_flows[num_ele - j];
                        }
                        all_pr[num_gatheredflow][num_ele] = pr_temp;
                    }
                }
            }

            // Save the brute-force outcomes into a file
            string output_str;
            ofstream out_param(pr_file_path);
            for(uint32_t num_gatheredflow = 0; num_gatheredflow <= max_num_gatheredflow; num_gatheredflow++) {
                output_str.clear();
                double * pr_ele_of_flows = all_pr[num_gatheredflow];
                for(uint32_t num_ele = 0; num_ele < MAX_NUM_ELE; num_ele++) {
                    stringstream stream;
                    stream << noshowpoint << setprecision(11) << pr_ele_of_flows[num_ele];
                    string s = stream.str();
                    output_str += s;
                    if(num_ele == MAX_NUM_ELE - 1) {
                        output_str += "\n";
                    }
                    else {
                        output_str += ",";
                    }
                }
                out_param << output_str;
            }
            out_param.close();
        }
        
        auto ** ans = new double * [th_num_gatheredflow+1];
        for(int i = 0; i <= max_num_gatheredflow; i++) {
            if(i <= th_num_gatheredflow) {
                ans[i] = all_pr[i];
            }
            else {
                delete [] all_pr[i];
            }
        }
        return ans;
    }

    vector<double> pr_ele_perunit(vector<double> &pr_num_flow_perunit, double ** pr_ele_of_gatheredflow) {
        vector<double> ans;
        for(uint32_t num_ele = 0; num_ele < MAX_NUM_ELE; num_ele++) {
            double sum_pr = 0;
            if(pr_num_flow_perunit.size() <= 200) {
                for(uint32_t num_gatheredflow = 0; num_gatheredflow < pr_num_flow_perunit.size(); num_gatheredflow++) {
                    sum_pr += pr_num_flow_perunit[num_gatheredflow] * pr_ele_of_gatheredflow[num_gatheredflow][num_ele];
                }
            }
            else{
                for(uint32_t num_gatheredflow = 0; num_gatheredflow < 201; num_gatheredflow++) {
                    sum_pr += pr_num_flow_perunit[num_gatheredflow] * pr_ele_of_gatheredflow[num_gatheredflow][num_ele];
                }
            }
            ans.push_back(sum_pr);
        }
        return ans;
    }

    double recursion_coup(uint32_t num_ele, uint32_t num_coup) {
        if(all_pr_coupon_of_ele[num_coup][num_ele] != -1) {
            return all_pr_coupon_of_ele[num_coup][num_ele];
        }
        else {
            double curr_pr = 0;
            if(num_coup > 0) {
                curr_pr = recursion_coup(num_ele - 1, num_coup) * (1 - c * p + num_coup * p)
                        + recursion_coup(num_ele - 1, num_coup - 1) * (c - num_coup + 1) * p;
            }
            else {
                curr_pr = recursion_coup(num_ele - 1, num_coup) * (1 - c * p);
            }
            all_pr_coupon_of_ele[num_coup][num_ele] = curr_pr;
            return curr_pr;
        }
    }

    vector<vector<double>> pr_coupon_of_ele() {
        const uint32_t recursion_deepth = MAX_NUM_ELE;
        for(int i = 0; i <= c; i++) {
            all_pr_coupon_of_ele[i] = new double [recursion_deepth+1];
            for(int j = 0; j < recursion_deepth+1; j++) {
                all_pr_coupon_of_ele[i][j] = -1;
            }
            all_pr_coupon_of_ele[i][0] = 0;
        }
        all_pr_coupon_of_ele[0][0] = 1;

        recursion_coup(recursion_deepth, c);
        vector<vector<double>> ans;
        ans.clear();
        for(int i = 0; i <= c; i++) {
            if(i >= c-1) {
                vector<double> temp(all_pr_coupon_of_ele[i], all_pr_coupon_of_ele[i] + recursion_deepth+1);
                ans.push_back(temp);
            }
            delete [] all_pr_coupon_of_ele[i];
        }
        return ans;
    }

    PrAnls_Res get_biases(vector<vector<double>> pr_coupon_of_ele, vector<double> pr_ele_per_unit) {
        vector<double> pr_all_coupon_of_min_ele;
        vector<double> pr_all_coupon_of_ele;
        double fullfilled_pr_perflow = 0;
        for(int i = 0; i < pr_coupon_of_ele[0].size()/2; i++) {
            if(i == 0) {
                pr_all_coupon_of_min_ele.push_back(pr_coupon_of_ele[1][i]);
            }
            else {
                pr_all_coupon_of_min_ele.push_back(pr_coupon_of_ele[0][i-1] * p);
            }
        }
        for(int i = 0; i < pr_coupon_of_ele[1].size()/2; i++) {
            double p = pr_coupon_of_ele[1][i] * pr_ele_per_unit[i];
            fullfilled_pr_perflow += p;
            pr_all_coupon_of_ele.push_back(p);
        }

        PrAnls_Res ans;
        double avg_ans = 0;
        double var_ans = 0;
        for(int i = 0; i < pr_all_coupon_of_min_ele.size(); i++) {
            double pr_temp = 0;
            for(int j = 0; j < i; j++) {
                pr_temp += pr_ele_per_unit[j];
            }
            avg_ans += i * pr_all_coupon_of_min_ele[i] * (1 - pr_temp);
        }
        avg_ans /= fullfilled_pr_perflow;
        ans.avg_consumed_ele = avg_ans;
        for(int i = 0; i < pr_all_coupon_of_min_ele.size(); i++) {
            double pr_temp = 0;
            for(int j = 0; j < i; j++) {
                pr_temp += pr_ele_per_unit[j];
            }
            var_ans += pow((i - avg_ans), 2) * pr_all_coupon_of_min_ele[i] * (1 - pr_temp);
        }
        var_ans /= fullfilled_pr_perflow;
        ans.var_consumed_ele = var_ans;

        vector<double> pr_all_coupon_with_min_ele;
        for(int i = 0; i < pr_all_coupon_of_min_ele.size(); i++) {
            double pr_temp = 0;
            for(int j = 0; j < i; j++) {
                pr_temp += pr_ele_per_unit[j];
            }
            pr_temp = (1 - pr_temp) * pr_all_coupon_of_min_ele[i] / fullfilled_pr_perflow;
            pr_all_coupon_with_min_ele.push_back(pr_temp);
        }
        double num_passed_ele = 0;
        for(int min_ele = 0; min_ele < pr_all_coupon_with_min_ele.size(); min_ele ++) {
            for(int ele = min_ele; ele < pr_ele_per_unit.size(); ele++) {
                num_passed_ele += (ele - min_ele) * pr_all_coupon_with_min_ele[min_ele] * pr_ele_per_unit[ele];
            }
        }
        num_passed_ele = num_passed_ele * m;
        ans.num_passed_ele = num_passed_ele;

        return ans;
    }

};


#endif // _COUPONFILTER_PRANLS_H
