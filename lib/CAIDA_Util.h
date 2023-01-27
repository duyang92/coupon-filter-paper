#ifndef _CAIDA_UTIL_H
#define _CAIDA_UTIL_H

#include "iostream"
#include "fstream"
#include "sys/stat.h"
#include "string"
#include "vector"

#include "./Basic_Util.h"

using namespace std;

#define NUM_SLICE_FREQ 1
#define NUM_SLICE_CARDI 3
#define NUM_SLICE_PERST 1

typedef struct {
    uint32_t source_addr;
    uint32_t target_addr;
} Pkt_Info;


template<bool unique_tag>
class CAIDA_Util
{
private:
    string caida_dir_path;
    vector<Pkt_Info> data_set;

public:
    CAIDA_Util() {
        if(!unique_tag) caida_dir_path = "../data/processed";
        else caida_dir_path = "../data/processed_unique";
        data_set.clear();
    }

    int read_caida_file_uint32(uint32_t num_slice, uint32_t start_index = 0) {
        data_set.clear();
        for(uint32_t slice_index = start_index; slice_index < start_index + num_slice; slice_index++) {
            string slice_path = caida_dir_path + "/0" + to_string(slice_index) + ".txt";
            struct stat buffer;
            if(stat(slice_path.c_str(), &buffer) != 0) {
                cout << "File does not exist!" << endl;
                exit(-1);
            }
            else {
                ifstream in_caida(slice_path, ios::binary);
                Pkt_Info temp;
                while (in_caida >> temp.source_addr && in_caida >> temp.target_addr) {
                    data_set.push_back(temp);
                }
                in_caida.close();
            }
        }
        return 0;
    }

    vector<Pkt_Info> get_caida_dataset() {
        return data_set;
    }

};


#endif // _CAIDA_UTIL_H
