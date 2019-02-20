/**
 * Lexicon is used to look up termID given a term. Thus, lexicon uses term as key, termID as value.
 * Dictionary in the indexer can also be called extended lexicon. It uses termID as key.
 */

#ifndef LEXICON_H
#define LEXICON_H

#include <string>
#include <vector>
#include <sstream>
#include "strless.hpp"

class Lexicon{
private:
    map<string, unsigned int, strless> lex;

public:
    void build_lexical();
    void display_lexical();
    unsigned int get_id(string term);
};

#endif
