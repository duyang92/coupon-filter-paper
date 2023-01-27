#ifndef _QUERYINTERFACE_H
#define _QUERYINTERFACE_H

#include "cstdint"
using namespace std;

class QueryInterface
{
public:
    virtual void to_offchip(uint64_t flow_id, uint64_t ele_id) = 0;

    virtual uint32_t query(uint64_t flow_id) = 0;
};

#endif // _QUERYINTERFACE_H
