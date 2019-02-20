#include "posting.hpp"

Posting::Posting (){}

Posting::~Posting (){}

Posting::Posting(unsigned int id, unsigned int d, unsigned int f, unsigned int p){
	termID = id;
	docID = d;
	fragID = f;
	pos = p;
}

nPosting::nPosting(){}

nPosting::~nPosting(){};

nPosting::nPosting (unsigned int id, unsigned int d, unsigned int f, unsigned int s){
    termID = id;
    docID = d;
    freq = f;
    sign = s;
}
