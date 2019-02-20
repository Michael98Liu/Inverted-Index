#include "comparison.hpp"
// For positional posting, first compare term ID, then, document ID,
// then fragment ID, then position in the fragment.
bool operator< (const Posting& p1, const Posting& p2) {
	if(p1.termID == p2.termID){
		if(p1.docID == p2.docID){
			if(p1.fragID == p2.fragID){
				return (p1.pos < p2.pos);
			}else{
				return (p1.fragID < p2.fragID);
			}
		}else{
			return (p1.docID < p2.docID);
		}
	}else{
		return (p1.termID < p2.termID);
	}
}

bool operator> (const Posting& p1, const Posting& p2) {
    if(p1.termID == p2.termID){
		if(p1.docID == p2.docID){
			if(p1.fragID == p2.fragID){
				return (p1.pos > p2.pos);
			}else{
				return (p1.fragID > p2.fragID);
			}
		}else{
			return (p1.docID > p2.docID);
		}
	}else{
		return (p1.termID > p2.termID);
	}
}

bool operator== (const Posting& p1, const Posting& p2){
	if(p1.termID == p2.termID && p1.docID == p2.docID && p1.fragID == p2.fragID && p1.pos == p2.pos) return true;
	else return false;
}

//For non-positinal positng, first compare the term Id, then document ID.

bool operator< (const nPosting& p1, const nPosting& p2) {
	if(p1.termID == p2.termID){
		return (p1.docID < p2.docID);
	}else{
		return (p1.termID < p2.termID);
	}
}

bool operator> (const nPosting& p1, const nPosting& p2) {
    if(p1.termID == p2.termID){
		return (p1.docID > p2.docID);
	}else{
		return (p1.termID > p2.termID);
	}
}

bool operator== (const nPosting& p1, const nPosting& p2){
	if(p1.termID == p2.termID && p1.docID == p2.docID) return true;
	else return false;
}

inline bool less_than_key::operator() (const Posting& p1, const Posting& p2){
    if(p1.termID == p2.termID){
    	if(p1.docID == p2.docID){
    		if(p1.fragID == p2.fragID){
    			return (p1.pos < p2.pos);
    		}else{
    			return (p1.fragID < p2.fragID);
    		}
    	}else{
    		return (p1.docID < p2.docID);
    	}
    }else{
    	return (p1.termID < p2.termID);
    }
}
