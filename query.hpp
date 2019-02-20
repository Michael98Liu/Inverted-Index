#include <vector>
#include <map>
#include <string>
#include "posting.hpp"
#include "meta.hpp"
#include "strless.hpp"

typedef std::map<std::string, unsigned int, strless> lexmap;

class Querior{
private:
    vector<listptr> ptrs;

public:
    std::vector<unsigned int> get_last_id();
    bool NextGEQ();
    std::vector<Posting> query(std::string terms);
};
