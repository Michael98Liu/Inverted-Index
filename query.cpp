#include <vector>
#include <map>
#include <string>
#include <sstream>
#include "query.hpp"
#include "strless.hpp"
#include "lexicon.hpp"

using namespace std;

bool Querior::NextGEQ(Lstptr* lp, unsigned int id){
    while( lp->currentID < id ){
        (*lp) ++;
    }
    
}

lstptr* Querior::openlist(string term, Indexer& ind){
    unsigned int termID = lex.get_id(word);
    vector<mDatanp> NPposting = ind.get_mdatanp(termID);//TODO: implement the method get_mdatanp
    Lstptr* lp = new Lstptr(NPposting);

}

vector<Posting> Querior::query(string terms){
    Lexicon lex;
    Indexer ind;

    stringstream lineStream(terms);
    string word;
    while( lineStream >> word ){
        openlist(word);
    }
}
