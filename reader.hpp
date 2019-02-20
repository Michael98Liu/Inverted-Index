/*
 * Contains three kinds of decoding function.
 * Given a file containing foward index and a ending position, read the file until ending postition without decoding.
 * Given a file and both starting and ending position, decode the bianry vector and return a vector of unsigned integer.
 * Given a binary vector, return a decoded unsigned integer vector.
 */

#ifndef READER_H
#define READER_H
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include "posting.hpp"
#include "meta.hpp"

class Reader{
public:

	static std::vector<char> read_com(std::ifstream& infile, long end_pos);

	static std::vector<unsigned int> VBDecode(std::ifstream& ifile, long start_pos = 0, long end_pos = 0);

	static std::vector<unsigned int> VBDecode(std::vector<char>& vec);


};

#endif
