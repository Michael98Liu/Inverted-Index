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
#include "strless.hpp"
#include "comparison.hpp"

#define NO_DOC 10 //temporary use
#define POSTING_LIMIT 500 //make sure doesn't exceed memory limit
#define INDEX "./test_data/compressedIndex"
#define INFO "./test_data/docInfo"
#define PDIR "./disk_index/positional/"
#define NPDIR "./disk_index/non_positional/"

using namespace std;

std::vector<char> Reader::read_com(ifstream& infile, long end_pos){//read compressed forward index
	char c;
	vector<char> result;
	while(infile.tellg() != end_pos){
        infile.get(c);
		result.push_back(c);
	}
    //the last one is not read in the loop
    infile.get(c);
    result.push_back(c);
	return result;
}

std::vector<unsigned int> Reader::VBDecode(ifstream& ifile, long start_pos, long end_pos){//ios::ate
	ifile.seekg(start_pos);
	char c;
	unsigned int num;
	int p;
	vector<unsigned int> result;
    if(end_pos == 0) end_pos = ifile.end;
	vector<char> vec = read_com(ifile, end_pos);

	for(vector<char>::iterator it = vec.begin(); it != vec.end(); it++){
		c = *it;
		bitset<8> byte(c);
		num = 0;
		p = 0;
		while(byte[7] == 1){
			byte.flip(7);
			num += byte.to_ulong()*pow(128, p);
			p++;
			it ++;
			c = *it;
			byte = bitset<8>(c);
		}
		num += (byte.to_ulong())*pow(128, p);

		result.push_back(num);
	}
	return result;
}

std::vector<unsigned int> Reader::VBDecode(vector<char>& vec){
	unsigned int num;
	vector<unsigned int> result;
	char c;
	int p;
	for(vector<char>::iterator it = vec.begin(); it != vec.end(); it++){
		c = *it;
		bitset<8> byte(c);
		num = 0;
		p = 0;
		while(byte[7] == 1){
			byte.flip(7);
			num += byte.to_ulong()*pow(128, p);
			p++;
			it ++;
			c = *it;
			byte = bitset<8>(c);
		}
		num += (byte.to_ulong())*pow(128, p);

		result.push_back(num);
	}
	return result;
}
