#ifndef _BASIC_UTIL_H
#define _BASIC_UTIL_H

#include "string"
#include "vector"
#include "arpa/inet.h"
#include <algorithm>

using namespace std;

void str_split(const string& s, vector<string>& sv, const char delim = ' ') {
    string::const_iterator start = s.begin();
    string::const_iterator end = s.end();
    string::const_iterator next = find(start, end, delim);
    while ( next != end ) {
        sv.push_back(string(start, next));
        start = next + 1;
        next = find(start, end, delim);
    }
    sv.push_back(string(start, next));
}

bool is_valid_ipv4_addr(const string& ip_addr) {
    struct sockaddr_in antelope;
    int source_status = inet_pton(AF_INET, ip_addr.c_str(), &(antelope.sin_addr));
    if(source_status == 1){
        return true;
    }
    else {
        return false;
    }
}

uint32_t ip_addr_to_int(string ip_addr) {
    struct sockaddr_in antelope;
    inet_aton(ip_addr.c_str(), &(antelope.sin_addr));
    return antelope.sin_addr.s_addr;
}

string int_to_ip_addr(uint32_t ip_val) {
    struct in_addr ip_addr = {ip_val};
    return inet_ntoa(ip_addr);
}

#endif // _BASIC_UTIL_H
