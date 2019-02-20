#ifndef LSTPTR_H
#define LSTPTR_H

#include <vector>
#include "meta.hpp"

class Lstptr{
public:
    Lstptr(std::vector<mDatanp>);
    void operator++ ();

private:
    long cptr;//current pointer
    unsigned int cval;//current value
    std::vector<long> block_size;
    std::vector<unsigned int> last_id;
    std::vector<long>::iterator sizeit;
    std::vector<unsigned int>::iterator lidit;//iterator for the last_id vector

};

#endif
