#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <map>
#include <string>
#include <cstdio>
#include <dirent.h>
#include <utility>

#include "reader.hpp"
#include "posting.hpp"
#include "meta.hpp"
#include "indexer.hpp"
#include "strless.hpp"
#include "comparison.hpp"
#include "lexicon.hpp"

#define NO_DOC 10 //when testing, only the first 10 documents are indexed
#define POSTING_LIMIT 500 //make sure doesn't exceed memory limit
#define INDEX "./test_data/compressedIndex"
#define INFO "./test_data/docInfo"
#define PDIR "./disk_index/positional/"
#define NPDIR "./disk_index/non_positional/"

using namespace std;

int main(){
	Indexer ind;
	Lexicon lex;
	lex.build_lexical();
	lex.display_lexical();
	cin.get();

	ind.start_compress();
	ind.display_dict();
	ind.display_meta();

    /*Querior q;
    string str;
    cin >> str;
    q.query(str, lexical);
    */

	return 0;
}
