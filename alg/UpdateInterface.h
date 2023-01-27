#ifndef _UPDATEINTERFACE_H
#define _UPDATEINTERFACE_H

#include "cstdint"
using namespace std;

class UpdateInterface
{
public:
    virtual void update(uint64_t flow_id, uint64_t ele_id) = 0;
};

#endif // _UPDATEINTERFACE_H
