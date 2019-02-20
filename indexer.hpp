/**
 * For building the static index, dynamic index, extended lexicon, and file metadata.
 */
#ifndef INDEXER_H
#define INDEXER_H

#include <vector>
#include <string>
#include <fstream>
#include <map>
#include "meta.hpp"
#include "posting.hpp"
#include "strless.hpp"
#include "externalpostings.hpp"

class Indexer{
private:
	map<unsigned int, std::pair<vector<mData>, vector<mDatanp>>> dict;
	map<string, vector<f_meta>, strless> filemeta;
	vector<Posting> p_index;
	vector<nPosting> np_index;

public:
	std::vector<std::string> read_directory( std::string path );

  	void update_f_meta(std::string s1, std::string s2);

	void update_t_meta(unsigned int termID, std::string file);

	void copy_and_paste(std::ifstream& ifile, std::ofstream& ofile, long start, long end);

	void merge_p(int indexnum);

	void merge_test();

	void write(std::vector<uint8_t> num, std::ofstream& ofile);

    bool check_contain(std::vector<std::string> v, std::string f);

	std::vector<char> read_com(std::ifstream& infile);

	std::vector<uint8_t> VBEncode(unsigned int num);

	std::vector<uint8_t> VBEncode(std::vector<unsigned int>& nums);

	std::vector<uint8_t> compress(std::vector<unsigned int>& field, int method, int sort, std::vector<uint8_t> &meta_data_biv, std::vector<unsigned int> &last_id_biv);

	std::vector<uint8_t> compress(std::vector<unsigned int>& field, int method, int sort, std::vector<uint8_t> &meta_data_biv);

	mData compress_p(std::string namebase, std::ofstream& ofile, f_meta& fm, std::vector<unsigned int>& v_docID, std::vector<unsigned int>& v_fragID, std::vector<unsigned int>& v_pos);

	void compress_p(std::vector<Posting>& pList, int indexnum = 0, char prefix = 'a');

	void start_compress();

	std::vector<uint8_t> compress_freq(std::vector<unsigned int>& field, int method, int sort, std::vector<uint8_t> &meta_data_biv);

	mDatanp compress_np(std::string namebase, std::ofstream& ofile, f_meta& fm, std::vector<unsigned int>& v_docID, std::vector<unsigned int>& v_freq, std::vector<unsigned int>& v_sign);

	void compress_np(std::vector<nPosting>& npList, int indexnum = 0, char prefix = 'a');

	void merge_np( int indexnum );

	std::vector<Posting> decompress_p(std::string namebase, unsigned int termID);

	std::vector<nPosting> decompress_np(std::string namebase, unsigned int termID);

	void update_p(vector<ExternPposting> external);

	void update_np(vector<ExternNPposting> external);

	void display_meta();

	void display_dict();

	std::vector<unsigned int> Indexer::decompress_np(std::string namebase, long start, long end);
};

#endif
