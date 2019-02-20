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
#include "indexer.hpp"
#include "reader.hpp"
#include "comparison.hpp"
#include "lexicon.hpp"
#include "externalpostings.hpp"

#define NO_DOC 10 //temporary use
#define POSTING_LIMIT 500 //make sure doesn't exceed memory limit
#define INDEX "./test_data/compressedIndex"
#define INFO "./test_data/docInfo"
#define PDIR "./disk_index/positional/"
#define NPDIR "./disk_index/non_positional/"

std::vector<std::string> Indexer::read_directory( std::string path ){
	/**
	 * List all the files in a directory.
	 *
	 * @param: the path of a directory
	 * @return: the names of all the files in this directory.
	 */
	std::vector <std::string> result;
	dirent* de;
	DIR* dp;
	errno = 0;
	dp = opendir( path.empty() ? "." : path.c_str() );
		if (dp){
    		while (true){
      			errno = 0;
      			de = readdir( dp );
      			if (de == NULL) break;
                string d_n(de->d_name);
      			result.push_back( d_n );
      		}
    		closedir( dp );
	    }
	return result;
}

void Indexer::update_f_meta(string s1, string s2){
	/* Delete the existence of two files in the file meta data map.
	 * Normally the two files has the same index number.
	 * Function is called after finnishing merging.
	 *
	 * @param: names(including directory of two files)
	 */
	filemeta.erase(s1);
	filemeta.erase(s2);
}

void Indexer::update_t_meta(unsigned int termID, string file){
	/* Update the meta data of a term.
	 * Delete the record of a file contains this term,
	 * because this file no longer contains it.
	 */
	for( vector<mData>::iterator it = dict[termID].first.begin(); it != dict[termID].first.end(); it++){
		if( it->filename == file ) {
            dict[termID].first.erase(it);
            break;
        }
	}

    for( vector<mDatanp>::iterator it = dict[termID].second.begin(); it != dict[termID].second.end(); it++){
		if( it->filename == file ) {
            dict[termID].second.erase(it);
            break;
        }
	}
}

void Indexer::copy_and_paste(ifstream& ifile, ofstream& ofile, long start, long end){
	/* Copy the compressed posting list and paste to
	 * a new file without decompressing given the start
	 * and end postion of such posting list.
	 */
	ifile.seekg(start);
	char c;
	while(ifile.tellg() != end){
		ifile.get(c);
		ofile << c;
	}
    //cout << "Copy and pasting finished." << endl;
}

void Indexer::merge_p(int indexnum){
    /**
	 * Merge two positional index.
	 */
	ifstream filez;
	ifstream filei;
	ofstream ofile;
    string pdir(PDIR);

	//determine the name of the output file, if "Z" file exists, than compressed to "I" file.
    char flag = 'Z';
	filez.open(pdir + "Z" + to_string(indexnum));
	filei.open(pdir + "I" + to_string(indexnum));

	ofile.open(pdir + "Z" + to_string(indexnum + 1), ios::app | ios::binary);
	if(ofile.tellp() != 0){
		ofile.close();
		ofile.open(pdir + "I" + to_string(indexnum + 1), ios::ate | ios::binary);
        flag = 'I';
	}

    cout << "Merging into " << flag << indexnum + 1 << "------------------------------------" << endl;

	string file1 = "Z" + to_string(indexnum);
	string file2 = "I" + to_string(indexnum);
	vector<f_meta>& v1 = filemeta[file1];
	vector<f_meta>& v2 = filemeta[file2];
	vector<f_meta>::iterator it1 = v1.begin();
	vector<f_meta>::iterator it2 = v2.begin();

	/**
	 * Go through the meta data of each file, do
	 * if there is a termID appearing in both, decode the part and merge
	 * else copy and paste the corresponding part of postinglist
	 * update the corresponding fileinfo of that termID
	 * assume that the posting of one term can be stored in memory
	 */
	while( it1 != v1.end() && it2 != v2.end() ){
		if( it1->termID == it2->termID ){
			//decode and merge
			//update meta data corresponding to the term
			vector<Posting> vp1 = decompress_p(file1, it1->termID);
            vector<Posting> vp2 = decompress_p(file2, it2->termID);
			vector<Posting> vpout; //store the sorted result

			//use NextGQ to write the sorted vector of Posting to disk
			vector<Posting>::iterator vpit1 = vp1.begin();
			vector<Posting>::iterator vpit2 = vp2.begin();
			while( vpit1 != vp1.end() && vpit2 != vp2.end() ){
				//NextGQ
				if( *vpit1 < *vpit2 ){
					vpout.push_back(*vpit1);
					vpit1 ++;
				}
				else if( *vpit1 > *vpit2 ){
					vpout.push_back(*vpit2);
					vpit2 ++;
				}
				else if ( *vpit1 == *vpit2 ){
					cout << "Error: same posting appearing in different indexes." << endl;
					break;
				}
			}
            while( vpit1 != vp1.end()){
                vpout.push_back(*vpit1);
                vpit1 ++;
            }
            while( vpit2 != vp2.end()){
                vpout.push_back(*vpit2);
                vpit2 ++;
            }

			compress_p(vpout, indexnum + 1, flag);
			it1 ++;
			it2 ++;
		}
		else if( it1->termID < it2->termID ){
			vector<Posting> vp = decompress_p(file1, it1->termID);
            compress_p(vp, indexnum + 1, flag);
			it1 ++;
		}
		else if( it1->termID > it2->termID ){
            vector<Posting> vp = decompress_p(file2, it2->termID);
            compress_p(vp, indexnum + 1, flag);
			it2 ++;
		}
	}

    /**
	 * TODO: decompress from the old index and then compress to
	 * the new one to update metadata is time-consuming
     * need to find a more efficient way to update metadata while tranfering positngs
	 */
	while (it1 != v1.end() ){
        vector<Posting> vp = decompress_p(file1, it1->termID);
        //update_t_meta(it1->termID, file1, dict);
        compress_p(vp, indexnum + 1, flag);
        it1 ++;
	}
	while (it2 != v2.end() ){
        vector<Posting> vp = decompress_p(file2, it2->termID);
        compress_p(vp, indexnum + 1, flag);
        it2 ++;
	}

	filez.close();
	filei.close();
	ofile.close();
    string filename1 = pdir + "Z" + to_string(indexnum);
    string filename2 = pdir + "I" + to_string(indexnum);

    for( vector<f_meta>::iterator it = filemeta[file1].begin(); it != filemeta[file1].end(); it ++){
        update_t_meta(it->termID, file1);
    }

    for( vector<f_meta>::iterator it = filemeta[file2].begin(); it != filemeta[file2].end(); it ++){
        update_t_meta(it->termID, file2);
    }

    update_f_meta(file1, file2);

    //deleting two files
    if( remove( filename1.c_str() ) != 0 )
        cout << "Error deleting file" << endl;

    if( remove( filename2.c_str() ) != 0 )
        cout << "Error deleting file" << endl;
}

bool Indexer::check_contain(vector<string> v, string f){
	//check if a vector contains an element
    for( vector<string>::iterator it = v.begin(); it != v.end(); it ++){
        if( *it == f) return true;
    }
    return false;
}

void Indexer::merge_test(){
    /**
	 * Test if there are two files of same index number on disk.
	 * If there is, merge them and then call merge_test again until
	 * all index numbers has only one file each.
	 */
	int indexnum = 0;
	string dir = string(PDIR);
    string npdir = string(NPDIR);
	vector<string> files = read_directory(dir);
    string fp = string("I") + to_string(indexnum);
    string fnp = string("L") + to_string(indexnum);

	while(check_contain(files, fp)){
		//if In exists already, merge In with Zn
        files.clear();
		merge_p(indexnum);
		indexnum ++;
        fp = string("I") + to_string(indexnum);
        files.clear();
        files = read_directory(dir);
	}
    indexnum = 0;
    vector<string> npfiles = read_directory(npdir);
    while(check_contain(npfiles, fnp)){
		//if Ln exists already, merge In with Xn
        npfiles.clear();
		merge_np(indexnum);
		indexnum ++;
        fnp = string("L") + to_string(indexnum);
        npfiles.clear();
        npfiles = read_directory(npdir);
	}
}

void Indexer::write(vector<uint8_t> num, ofstream& ofile){
	/**
	 * Write the compressed posting to file byte by byte.
	 */
	for(vector<uint8_t>::iterator it = num.begin(); it != num.end(); it++){
		ofile.write(reinterpret_cast<const char*>(&(*it)), 1);
	}
}

std::vector<char> Indexer::read_com(ifstream& infile){
	/**
	 * Read compressed posting from file byte by byte.
	 */
	char c;
	vector<char> result;
	while(infile.get(c)){
		result.push_back(c);
	}
	return result;
}

std::vector<uint8_t> Indexer::VBEncode(unsigned int num){
	/**
	 * Variable byte encoding.
	 *
	 * @param: an umsigned interger to be compressed
	 * @return: a vector of bytes
	 */
	vector<uint8_t> result;
	uint8_t b;
	while(num >= 128){
		int a = num % 128;
		bitset<8> byte(a);
		byte.flip(7);
		num = (num - a) / 128;
		b = byte.to_ulong();
		result.push_back(b);
	}
	int a = num % 128;
	bitset<8> byte(a);
	b = byte.to_ulong();
	result.push_back(b);
	return result;
}

std::vector<uint8_t> Indexer::VBEncode(vector<unsigned int>& nums){
	/**
	 * Encode a vector of unsigned intergers.
	 */
	vector<uint8_t> biv;
	vector<uint8_t> result;
	for( vector<unsigned int>::iterator it = nums.begin(); it != nums.end(); it ++){
		biv = VBEncode(*it);
		result.insert(result.end(), biv.begin(), biv.end());
	}
	return result;
}

vector<uint8_t> Indexer::compress(std::vector<unsigned int>& field, int method, int sort, vector<uint8_t> &meta_data_biv, vector<unsigned int> &last_id_biv){
	/**
	 * Compress the document ID field and store the last ID of each block to last_id_biv,
	 * and store the length of each block to meta_data_biv.
	 */
	std::vector<unsigned int> block;
	std::vector<unsigned int>::iterator it = field.begin();
	std::vector<uint8_t> field_biv;
	std::vector<uint8_t> biv;

	if(method){
		unsigned int prev = 0;
		int size_block;
		while(it != field.end()){
			size_block = 0;
			block.clear();

			while(size_block < 64 && it != field.end()){
				block.push_back(*it - prev);
				prev = *it;
				size_block ++;
				it ++;
			}
			biv = VBEncode(block);
			last_id_biv.push_back(prev);//the last element of every block needs to be stored
			field_biv.insert(field_biv.end(), biv.begin(), biv.end());
			meta_data_biv.push_back(biv.size());//meta data stores the number of bytes after compression
		}
		return field_biv;
	}
	else{
		return field_biv;
	}
}

vector<uint8_t> Indexer::compress(std::vector<unsigned int>& field, int method, int sort, vector<uint8_t> &meta_data_biv){
	/**
	 * Compress fragment ID and positions, and store the length of each block to meta_data_biv.
	 * meta_data_biv here is acutally not a binary vector.
	 * TODO: the meta_data_biv should not be binary vector, instead it should be vector of unsigned int.
	 */
	std::vector<unsigned int> block;
	std::vector<unsigned int>::iterator it = field.begin();
	std::vector<uint8_t> field_biv;
	std::vector<uint8_t> biv;

	if(method){
		int prev;
		int size_block;
		while(it != field.end()){
			size_block = 0;
			block.clear();
			prev = 0;//the first element of every block needs to be renumbered

			while(size_block < 64 && it != field.end()){
				block.push_back(*it - prev);
				prev = *it;
				size_block ++;
				it ++;
			}
			biv = VBEncode(block);

			field_biv.insert(field_biv.end(), biv.begin(), biv.end());
			meta_data_biv.push_back(biv.size());//meta data stores the number of bytes after compression
		}
		return field_biv;
	}
	else{
		return field_biv;
	}
}

mData Indexer::compress_p(string namebase, ofstream& ofile, f_meta& fm, std::vector<unsigned int>& v_docID, std::vector<unsigned int>& v_fragID, std::vector<unsigned int>& v_pos){
	/**
	 * Write compressed positional postings to disk and store the corresponding start
	 * and end position to metadata of the term.
	 */
	string filename = string(PDIR) + namebase;
	int method = 1;// 1: Variable Bytes Encoding

	std::vector<unsigned int> v_last_id;
	std::vector<uint8_t> docID_biv;
	std::vector<uint8_t> fragID_biv;
	std::vector<uint8_t> pos_biv;

	std::vector<uint8_t> last_id_biv;
	std::vector<uint8_t> size_doc_biv;
	std::vector<uint8_t> size_frag_biv;
	std::vector<uint8_t> size_pos_biv;

	docID_biv = compress(v_docID, method, 1, size_doc_biv, v_last_id);
	last_id_biv = VBEncode(v_last_id);

	fragID_biv = compress(v_fragID, method, 0, size_frag_biv);
	pos_biv = compress(v_pos, method, 0, size_pos_biv);

	mData meta;
    meta.filename = namebase;
	meta.comp_method = method;

	fm.start_pos = ofile.tellp();
    meta.start_pos = ofile.tellp();
	write(last_id_biv, ofile);

	meta.meta_doc_start = ofile.tellp();
	write(size_doc_biv, ofile);

	meta.meta_frag_start = ofile.tellp();
	write(size_frag_biv, ofile);

	meta.meta_pos_start = ofile.tellp();
	write(size_pos_biv, ofile);

	meta.posting_start = ofile.tellp();
	write(docID_biv, ofile);

	meta.frag_start = ofile.tellp();
	write(fragID_biv, ofile);

	meta.pos_start = ofile.tellp();
	write(pos_biv, ofile);

	fm.end_pos = ofile.tellp();
    meta.end_pos = ofile.tellp();

	return meta;
}

void Indexer::compress_p(vector<Posting>& pList, int indexnum, char prefix){
	/**
	 * Compress positional index.
	 *
	 * @param: forward index of same termID
	 */
	ofstream ofile;//positional inverted index
    string pdir(PDIR);
    string namebase;
    string filename;
    if(prefix == 'a'){
        filename = pdir + "Z" + to_string(indexnum);
    	ofile.open(filename, ofstream::app | ofstream::binary);

    	if(ofile.tellp() != 0){
            cout << filename << " already exists." << endl;
    		ofile.close();
    		filename = pdir + "I" + to_string(indexnum);
            ofile.open(filename, ios::ate | ios::binary);
            namebase = string("I") + to_string(indexnum);
    	}else{
            namebase = string("Z") + to_string(indexnum);
        }
    }else{
        filename = pdir + prefix + to_string(indexnum);
    	ofile.open(filename, ofstream::app | ofstream::binary);
        namebase = prefix+ to_string(indexnum);
    }

    if (ofile.is_open()){
        std::vector<unsigned int> v_docID;
    	std::vector<unsigned int> v_fragID;
    	std::vector<unsigned int> v_pos;
    	mData mmData;
    	f_meta fm;
    	unsigned int num_of_p = 0;//number of posting of a certain term

    	unsigned int currID = pList[0].termID;//the ID of the term that is currently processing
        vector<Posting>::iterator it = pList.begin();
    	while( it != pList.end() ){
            //cout << it->termID << ' ' << it->docID << ' ' << it->fragID << ' ' << it->pos << endl;
    		while(it->termID == currID && it != pList.end()){
    			v_docID.push_back(it->docID);
    			v_fragID.push_back(it->fragID);
    			v_pos.push_back(it->pos);
    			it ++;
    			num_of_p ++;
    		}
    		fm.termID = currID;
    		mmData = compress_p(namebase, ofile, fm, v_docID, v_fragID, v_pos);
    		mmData.num_posting = num_of_p;
    		filemeta[namebase].push_back(fm);

    		//add mmdata to the dictionary of corresponding term
    		dict[currID].first.push_back(mmData);
            currID = it->termID;

    		num_of_p = 0;
    		v_docID.clear();
    		v_fragID.clear();
    		v_pos.clear();
    	}
    	ofile.close();
    }else{
        cerr << "File cannot be opened." << endl;
    }
}

void Indexer::start_compress(){
	/**
	 * The very first function to call.
	 */
	ifstream index;
	ifstream info;
	index.open(INDEX);
	info.open(INFO);
	string line;
	string value;
	vector<string> vec;
	char c;
	int p;
	int num;

	for(int i = 0; i < NO_DOC; i ++){
		//For every forward index of document, do
		vec.clear();
		getline(info, line);//read docInfo
		stringstream lineStream(line);
		unsigned int pos = 0;

		while(lineStream >> value){
			vec.push_back(value);
		}

		index.seekg(stoi(vec[2]));
		while(index.tellg() != (stoi(vec[2]) + stoi(vec[3]))){
			//While reading the same document, do
			index.get(c);
			bitset<8> byte(c);
			num = 0; // store decoding termID
			p = 0;//power
			while(byte[7] == 1){
				byte.flip(7);
				num += byte.to_ulong()*pow(128, p);
				p++;
				index.get(c);
				byte = bitset<8>(c);
			}
			num += (byte.to_ulong())*pow(128, p);
			pos ++;

			Posting p(num, stoul(vec[1]), 0, pos);
			p_index.push_back(p);


			if (p_index.size() == POSTING_LIMIT){
				//When the vector of posting list can fit into memory, do
                cout << "Memory limit reaches." << endl;
				std::sort(p_index.begin(), p_index.end());

                //Generate np_index
                unsigned int prevTermID = p_index[0].termID;
                unsigned int prevDocID = p_index[0].docID;
                unsigned int freq = 0;
                vector<Posting>::iterator it = p_index.begin();
                while(it != p_index.end()){
                    while( it->termID == prevTermID && it != p_index.end()){
                        while( it->docID == prevDocID && it != p_index.end()){
                            freq++;
                            it ++;
                        }
                        nPosting p(prevTermID, prevDocID, freq);
                        cout << prevTermID << ' '<< prevDocID << ' ' << freq << endl;
                        freq = 0;
                        prevDocID = it->docID;
                        np_index.push_back(p);
                    }
                    if(it!=p_index.end()){
                        prevTermID = it->termID;
                    }
                    else break;
                }
                compress_np(np_index);
				compress_p(p_index);
                merge_test();
				p_index.clear();
                np_index.clear();
			}
		}
	}

	index.close();
	info.close();
}

vector<uint8_t> Indexer::compress_freq(std::vector<unsigned int>& field, int method, int sort, vector<uint8_t> &meta_data_biv){
	/* Compress a field which doesn't require delta coding.
	 * In this case, frequency. Frequency cannot be delta coded because it is not strictly increasing.
	 */
	std::vector<unsigned int> block;
	std::vector<unsigned int>::iterator it = field.begin();
	std::vector<uint8_t> field_biv;
	std::vector<uint8_t> biv;

	if(method){

		int size_block;
		while(it != field.end()){
			size_block = 0;
			block.clear();

			while(size_block < 64 && it != field.end()){
                //cout <<"Freq " <<  *it << endl;
				block.push_back(*it);
				size_block ++;
				it ++;
			}
			biv = VBEncode(block);

			field_biv.insert(field_biv.end(), biv.begin(), biv.end());
			meta_data_biv.push_back(biv.size());//meta data stores the number of bytes after compression
		}
		return field_biv;
	}
	else{
		return field_biv;
	}
}

mDatanp Indexer::compress_np(string namebase, ofstream& ofile, f_meta& fm, std::vector<unsigned int>& v_docID, std::vector<unsigned int>& v_freq, std::vector<unsigned int>& v_sign){
	/**
	 * Writing compressed non-positional postings to disk and store the starting and ending positions.
	 * Similar to compress_p.
	 */

	string filename = string(NPDIR) + namebase;
	int method = 1;

	std::vector<unsigned int> v_last_id;
	std::vector<uint8_t> docID_biv;
	std::vector<uint8_t> freq_biv;
    std::vector<uint8_t> sign_biv;

	std::vector<uint8_t> last_id_biv;
	std::vector<uint8_t> size_doc_biv;
	std::vector<uint8_t> size_freq_biv;

	docID_biv = compress(v_docID, method, 1, size_doc_biv, v_last_id);
	last_id_biv = VBEncode(v_last_id);

	freq_biv = compress_freq(v_freq, method, 0, size_freq_biv);

    //compress sign vector
    vector<unsigned int>::iterator it = v_sign.begin();
    int num;
    uint8_t b;
    bitset<8> byte;
    while( it != v_sign.end() ){
        num = 0;
        while(num != 8 && it != v_sign.end()){
            if( *it == 1 )
                byte.flip( num );
            num ++;
            it ++;
        }
        b = byte.to_ulong();
        sign_biv.push_back(b);
    }

	mDatanp meta;
    meta.filename = namebase;
	meta.comp_method = method;

	fm.start_pos = ofile.tellp();
    meta.start_pos = ofile.tellp();
	write(last_id_biv, ofile);

	meta.meta_doc_start = ofile.tellp();
	write(size_doc_biv, ofile);

	meta.meta_freq_start = ofile.tellp();
	write(size_freq_biv, ofile);

	meta.posting_start = ofile.tellp();
	write(docID_biv, ofile);

	meta.freq_start = ofile.tellp();
	write(freq_biv, ofile);

	meta.sign_start = ofile.tellp();
	write(sign_biv, ofile);

	fm.end_pos = ofile.tellp();
    meta.end_pos = ofile.tellp();

	return meta;
}

void Indexer::compress_np(vector<nPosting>& npList, int indexnum, char prefix){
	/**
	 * Open a file to write to and store the metadata of a term.
	 */
    ofstream ofile;//non-positional inverted index
    string pdir(NPDIR);
    string namebase;
    string filename;
    if(prefix == 'a'){
        filename = pdir + "X" + to_string(indexnum);
    	ofile.open(filename, ofstream::app | ofstream::binary);

    	if(ofile.tellp() != 0){
            cout << filename << " already exists." << endl;
    		ofile.close();
    		filename = pdir + "L" + to_string(indexnum);
            ofile.open(filename, ios::ate | ios::binary);
            namebase = string("L") + to_string(indexnum);
    	}else{
            namebase = string("X") + to_string(indexnum);
        }
    }else{
        filename = pdir + prefix + to_string(indexnum);
    	ofile.open(filename, ofstream::app | ofstream::binary);
        namebase = prefix+ to_string(indexnum);
    }

    if (ofile.is_open()){
        //cout << "Compressing and writing to " << namebase << endl;

        std::vector<unsigned int> v_docID;
    	std::vector<unsigned int> v_freq;
        std::vector<unsigned int> v_sign;
    	mDatanp mmDatanp;
    	f_meta fm;

    	unsigned int currID = npList[0].termID;//the ID of the term that is currently processing
        vector<nPosting>::iterator it = npList.begin();
    	while( it != npList.end() ){
            //cout << it->termID << ' ' << it->docID << ' ' << it->fragID << ' ' << it->pos << endl;
    		while(it->termID == currID && it != npList.end()){
    			v_docID.push_back(it->docID);
    			v_freq.push_back(it->freq);
                v_sign.push_back(it->sign);
    			it ++;
    		}
    		fm.termID = currID;
            //cout << "Current term " << currID << endl;
    		mmDatanp = compress_np(namebase, ofile, fm, v_docID, v_freq, v_sign);
    		filemeta[namebase].push_back(fm);
            //cout << "This file contains: " << fm.termID << endl;

    		//add mmdata to the dictionary of corresponding term
    		dict[currID].second.push_back(mmDatanp);
            currID = it->termID;

    		v_docID.clear();
    		v_freq.clear();
    		v_sign.clear();
    	}

    	ofile.close();

        //merge_test(filemeta, dict);//see if need to merge
    }else{
        cerr << "File cannot be opened." << endl;
    }
}

void Indexer::merge_np(int indexnum){
    /**
	 * Merge non-positional index.
	 */
	ifstream filez;
	ifstream filei;
	ofstream ofile;
    string pdir(NPDIR);
    char flag = 'X';//determine the name of the output file
	filez.open(pdir + "X" + to_string(indexnum));
	filei.open(pdir + "L" + to_string(indexnum));

	ofile.open(pdir + "X" + to_string(indexnum + 1), ios::app | ios::binary);
	if(ofile.tellp() != 0){
        cout << "cannot merge to " << flag << indexnum + 1 << endl;
		ofile.close();
		ofile.open(pdir + "L" + to_string(indexnum + 1), ios::ate | ios::binary);
        flag = 'L';
	}

    cout << "Merging into " << flag << indexnum + 1 << "------------------------------------" << endl;

	string file1 = "X" + to_string(indexnum);
	string file2 = "L" + to_string(indexnum);
	vector<f_meta>& v1 = filemeta[file1];
	vector<f_meta>& v2 = filemeta[file2];
	vector<f_meta>::iterator it1 = v1.begin();
	vector<f_meta>::iterator it2 = v2.begin();

	/**
	 * Go through the meta data of each file, do
	 * if there is a termID appearing in both, decode the part and merge
	 * else copy and paste the corresponding part of postinglist
	 * update the corresponding fileinfo of that termID
	 * assume that the posting of one term can be stored in memory
	 */
	while( it1 != v1.end() && it2 != v2.end() ){
        //cout << it1->termID << ' ' << it2->termID << endl;
		if( it1->termID == it2->termID ){
			//decode and merge
			//update meta data corresponding to the term
			vector<nPosting> vp1 = decompress_np(file1, it1->termID);
            vector<nPosting> vp2 = decompress_np(file2, it2->termID);
			vector<nPosting> vpout; //store the sorted result

			//use NextGQ to write the sorted vector of Posting to disk
			vector<nPosting>::iterator vpit1 = vp1.begin();
			vector<nPosting>::iterator vpit2 = vp2.begin();
			while( vpit1 != vp1.end() && vpit2 != vp2.end() ){
				//NextGQ
				if( *vpit1 < *vpit2 ){
					vpout.push_back(*vpit1);
					vpit1 ++;
				}
				else if( *vpit1 > *vpit2 ){
					vpout.push_back(*vpit2);
					vpit2 ++;
				}
				else if ( *vpit1 == *vpit2 ){
                    vpout.push_back(*vpit1);
                    vpout.push_back(*vpit2);
					vpit1 ++;
                    vpit2 ++;
					break;
				}
			}
            while( vpit1 != vp1.end()){
                vpout.push_back(*vpit1);
                vpit1 ++;
            }
            while( vpit2 != vp2.end()){
                vpout.push_back(*vpit2);
                vpit2 ++;
            }
			compress_np(vpout, indexnum + 1, flag);
			it1 ++;
			it2 ++;
		}
		else if( it1->termID < it2->termID ){
			vector<nPosting> vp = decompress_np(file1, it1->termID);
            compress_np(vp, indexnum + 1, flag);
			it1 ++;
		}
		else if( it1->termID > it2->termID ){
            vector<nPosting> vp = decompress_np(file2, it2->termID);
            compress_np(vp, indexnum + 1, flag);
			it2 ++;
		}
	}

    /**
	 * TODO: decompress from the old index and then compress to the new one to update metadata is time-consuming
     * need to find a more efficient way to update metadata while tranfering positngs
	 */
	while (it1 != v1.end() ){
        vector<nPosting> vp = decompress_np(file1, it1->termID);
        compress_np(vp, indexnum + 1, flag);
        it1 ++;
	}
	while (it2 != v2.end() ){
        vector<nPosting> vp = decompress_np(file2, it2->termID);
        compress_np(vp, indexnum + 1, flag);
        it2 ++;
	}

	filez.close();
	filei.close();
	ofile.close();
    string filename1 = pdir + "X" + to_string(indexnum);
    string filename2 = pdir + "L" + to_string(indexnum);

    for( vector<f_meta>::iterator it = filemeta[file1].begin(); it != filemeta[file1].end(); it ++){
        update_t_meta(it->termID, file1);
    }

    for( vector<f_meta>::iterator it = filemeta[file2].begin(); it != filemeta[file2].end(); it ++){
        update_t_meta(it->termID, file2);
    }

    update_f_meta(file1, file2);

    //deleting two files
    if( remove( filename1.c_str() ) != 0 )
        cout << "Error deleting file" << endl;

    if( remove( filename2.c_str() ) != 0 )
        cout << "Error deleting file" << endl;
}

std::vector<Posting> Indexer::decompress_p(string namebase, unsigned int termID){
	/**
	 * Decompress positional postings.
	 * First read each field from disk and pass the bianry vector to decode.
	 * Finally add the terms up since we used delta encoding before.
	 */
	Reader r;

	ifstream ifile;
    string filename = string(PDIR) + namebase;
	ifile.open(filename, ios::binary);
    vector<Posting> result;

    if(ifile.is_open()){
        //cout << namebase << " Opened for Decompressing" << endl;
    	char c;
    	vector<char> readin;

    	vector<unsigned int> docID;
    	vector<unsigned int> fragID;
    	vector<unsigned int> pos;

        vector<mData>& mvec = dict[termID].first;
        vector<mData>::iterator currMVec;

        for( currMVec = mvec.begin(); currMVec != mvec.end(); currMVec ++){
            if( currMVec->filename == namebase )
                break;
        }

    	ifile.seekg(currMVec->posting_start);
    	while(ifile.tellg() != currMVec->frag_start){
    		ifile.get(c);
    		readin.push_back(c);
    	}
        docID = r.VBDecode(readin);
        readin.clear();

    	ifile.seekg(currMVec->frag_start);
    	while(ifile.tellg() != currMVec->pos_start){
    		ifile.get(c);
    		readin.push_back(c);
    	}
        fragID = r.VBDecode(readin);
        readin.clear();

    	ifile.seekg(currMVec->pos_start);
    	while(ifile.tellg() != currMVec->end_pos){
    		ifile.get(c);
    		readin.push_back(c);
    	}
        pos = r.VBDecode(readin);
        readin.clear();

    	vector<unsigned int>::iterator itdoc = docID.begin();
    	vector<unsigned int>::iterator itfrag = fragID.begin();
    	vector<unsigned int>::iterator itpos = pos.begin();

        unsigned int prevdoc =0;
        unsigned int prevfrag = 0;
        unsigned int prevpos = 0;

        int posting_num = 0;

    	while(itdoc != docID.end()){
            if(posting_num == 64){
                //keep track of block
                prevfrag = 0;
                prevpos = 0;
                posting_num = 0;
            }

            Posting p(termID, (*itdoc + prevdoc), (prevfrag+*itfrag), (prevpos+*itpos));
            //cout << termID << ' ' <<*itdoc << ' '<< *itfrag << ' ' << *itpos << endl;
            prevdoc = *itdoc + prevdoc;
            prevfrag = *itfrag + prevfrag;
            prevpos = *itpos + prevpos;
            result.push_back(p);
            itdoc ++;
            itfrag ++;
            itpos ++;
            posting_num ++;

    	}
    }else{
        cerr << "File cannot be opened." << endl;
    }
    return result;
}

std::vector<nPosting> Indexer::decompress_np(string namebase, unsigned int termID){
    //cout << "Decompressing np" << endl;
	Reader r;
	ifstream ifile;
    string filename = string(NPDIR) + namebase;
	ifile.open(filename, ios::binary);
    vector<nPosting> result;

    if(ifile.is_open()){
        //cout << namebase << " Opened for Decompressing" << endl;
    	char c;
    	vector<char> readin;

    	vector<unsigned int> docID;
    	vector<unsigned int> freq;
    	vector<unsigned int> sign;

        vector<mDatanp>& mvec = dict[termID].second;
        vector<mDatanp>::iterator currMVec;

        for( currMVec = mvec.begin(); currMVec != mvec.end(); currMVec ++){
            //cout << currMVec->filename << endl;
            //cout << currMVec->posting_start << ' ' << currMVec->frag_start << ' ' << currMVec->pos_start << endl;
            if( currMVec->filename == namebase )
                break;
        }

    	ifile.seekg(currMVec->posting_start);
    	while(ifile.tellg() != currMVec->freq_start){
    		ifile.get(c);
    		readin.push_back(c);
    	}
        docID = r.VBDecode(readin);
        readin.clear();

    	ifile.seekg(currMVec->freq_start);
    	while(ifile.tellg() != currMVec->sign_start){
    		ifile.get(c);
    		readin.push_back(c);
    	}
        freq = r.VBDecode(readin);
        readin.clear();

    	ifile.seekg(currMVec->sign_start);
    	while(ifile.tellg() != currMVec->end_pos){
            //cout << ifile.tellg() << endl;
            //cout << currMVec->end_pos << endl;
    		ifile.get(c);
    		readin.push_back(c);
    	}
        //decompress sign

        for( vector<char>::iterator it = readin.begin(); it != readin.end(); it ++ ){
            bitset<8> byte(*it);
            unsigned int ui;
            for( int i = 0; i < 8; i ++ ){
                ui = byte[i];
                sign.push_back(ui);
            }
        }
        readin.clear();

    	vector<unsigned int>::iterator itdoc = docID.begin();
    	vector<unsigned int>::iterator itfreq = freq.begin();
    	vector<unsigned int>::iterator itsign = sign.begin();

        unsigned int prevdoc =0;
        //cout << "In " << filename << ", there are "<< endl;
    	while(itdoc != docID.end()){
            nPosting p(termID, (*itdoc + prevdoc), *itfreq, *itsign);
            //cout << termID << ' ' << *itdoc + prevdoc << ' '<< *itfreq << ' ' << *itsign << endl;
            prevdoc = *itdoc + prevdoc;
            result.push_back(p);
            itdoc ++;
            itfreq ++;
            itsign ++;
    	}
    }else{
        cerr << "File cannot be opened." << endl;
    }
    return result;
}

vector<unsigned int> Indexer::decompress_np(string namebase, long start, long end){
	/**
	 * Decompress the posting list given the file and start & end position.
	*/
	Reader r;
	ifstream ifile;
    string filename = string(NPDIR) + namebase;
	ifile.open(filename, ios::binary);

    if(ifile.is_open()){
        //cout << namebase << " Opened for Decompressing" << endl;
    	char c;
    	vector<char> readin;
    	vector<unsigned int> field;

    	ifile.seekg(start);
    	while(ifile.tellg() != end){
    		ifile.get(c);
    		readin.push_back(c);
    	}
        field = r.VBDecode(readin);
	}
	return field;
}

void Indexer::update_p(vector<ExternPposting> external){
	/**
	 * Convert external posting to internal posting.
	 */
	unsigned int termID;
	Lexicon lex;
	for( vector<ExternPposting>::iterator it = external.begin(); it != external.end(); it++){
		termID = lex.get_id(it->term);
		Posting p(termID, it->docID, it->fragID, it->pos);
		p_index.push_back(p);
		if( p_index.size() == POSTING_LIMIT ){
			cout << "Memory limit reaches." << endl;
			std::sort(p_index.begin(), p_index.end());
			compress_p(p_index);
			merge_test();//see if need to merge
			p_index.clear();
		}
	}
}

void Indexer::update_np(vector<ExternNPposting> external){
	/**
	 * Update non_positional index.
	 */
	unsigned int termID;
	unsigned int freq;
	unsigned int sign;
	Lexicon lex;
	for( vector<ExternNPposting>::iterator it = external.begin(); it != external.end(); it++){
		termID = lex.get_id(it->term);
		if( it->freq >= 0 ){
			freq = it->freq;
			sign = 1;
		}else{
			freq = -it->freq;
			sign = 0;
		}
		nPosting p(termID, it->docID, freq, sign);
		np_index.push_back(p);
		if( np_index.size() == POSTING_LIMIT ){
			cout << "Memory limit reaches." << endl;
			std::sort(np_index.begin(), np_index.end());
			compress_np(np_index);
			merge_test();//see if need to merge
			np_index.clear();
		}
	}
}

void Indexer::display_dict(){
	/**
	 * Display the content of dictionary.
	 */
	for( map<unsigned int, pair<vector<mData>, vector<mDatanp>>>::iterator it = dict.begin(); it != dict.end(); it ++){
        cout << it->first << endl;
        vector<mData> vec = it->second.first;
        for( vector<mData>::iterator ite = vec.begin(); ite != vec.end(); ite ++){
            cout << ite->filename << ' ' << ite->start_pos << ' ';
        }
        cout << endl;
    }
}

void Indexer::display_meta(){
	/**
	 * Display content of metadata/
	 */
	for( map<string, vector<f_meta>, strless>::iterator it = filemeta.begin(); it != filemeta.end(); it++){
        cout << it->first << endl;
        vector<f_meta> vec = it->second;
        for( vector<f_meta>::iterator ite = vec.begin(); ite != vec.end(); ite++){
            cout << ite->termID << ' ';
        }
        cout << endl;
    }
}
