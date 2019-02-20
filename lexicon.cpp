#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <map>
#include "lexicon.hpp"
#include "strless.hpp"
using namespace std;

typedef map<string, unsigned int, strless> lexmap;
void Lexicon::build_lexical(){
    /**
     * Read term, termID pairs from file and build a map structure to store it.
     */
    int count = 0;
    string line;
    string term;
    string ID;
    ifstream ifile;
    ifile.open("./test_data/termIDs");

    while(getline(ifile, line)){
        count ++;
        stringstream lineStream(line);
        lineStream >> ID >> term;
        lex[term] = stoi(ID);
        cout << '\r' << count << " Finished.";
        if( count == 1000000) break;
    }
}

void Lexicon::display_lexical(){
    for(lexmap::iterator it = lex.begin(); it != lex.end(); ++it){
        cout << it->first << ' ' << it->second << endl;
    }
}

unsigned int Lexicon::get_id(string term){
    return lex[term];
}
