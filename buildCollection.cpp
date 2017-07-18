#include <iostream>
#include <fstream>
#include <boost/tokenizer.hpp>
#include <string>
#include <vector>
#include "dirent.h"
using namespace std;
using namespace boost;

class dictionary{
private:
	struct vocabulary{
		string term;
		int termID;
		long location; //starto location in the posting list
		int length; //length of the block in the posting list
	};

	vector<vocabulary> dict;
	int termID = 0;

public:
	vocabulary* find(string term){
		for(vector<vocabulary>::iterator it = dict.begin(); it != dict.end(); ++it){
			if(it->term == term){
				vocabulary* voc = new vocabulary;
				voc->term = term;
				voc->termID = it->termID;
				voc->location = it->location;
				voc->length = it->length;
				return voc;
			}
		}
		return nullptr;
	}

	vocabulary *find(int id){ //return a pointer to a vocabulary struct
		for(vector<vocabulary>::iterator it = dict.begin(); it != dict.end(); ++it){
			if(it->termID == id){
				vocabulary* voc = new vocabulary;
				voc->term = it->term;
				voc->termID = id;
				voc->location = it->location;
				voc->length = it->length;
				return voc;
			}
		}
		return nullptr;
	}

	void add(string term, int termID){
		dict.push_back({term, termID, 0, 0});
	}

	int update(string voc){
		if(!find(voc)){
			termID ++;
			add(voc, termID);
			return termID;
		}
	}

	void update(string term, long loc, int len){
		vocabulary* dicvoc = find(term);
		dicvoc->location = loc;
		dicvoc->length = len;
	}

	

	void display(){
		for(vector<vocabulary>::iterator it = dict.begin(); it != dict.end(); ++it){
			cout << it->term << ' ' << it->termID << endl;
		}
	}

	
};

class index{
private:
	struct postingList{
		int termID;
		vector<unsigned int> postings;
		vector<unsigned int> pos;
	};

	vector<postingList> ind;

public:
	void update(int termID, unsigned int docID, unsigned int position){
		vector<postingList>::iterator findterm = find(termID);
		if (findterm != ind.end()) {
			(findterm->postings).push_back(docID);
			(findterm->pos).push_back(position);
			// cout << findterm->term << endl;
			// for(vector<unsigned int>::iterator it2 = (findterm->postings).begin(); it2 != (findterm->postings).end(); ++it2){
			// 	cout << *it2 << ' ';
			// }
			// cout << endl;
		}
		else {
			postingList *posting = new postingList;
			posting->termID = termID;
			(posting->postings).push_back(docID);
			(posting->pos).push_back(position);
			ind.push_back(*posting);
		}
	}

	vector<postingList>::iterator find(int termID){
		vector<postingList>::iterator it;
		for(it = ind.begin(); it != ind.end(); ++ it){
			if(it->termID == termID){
				return it;
			}
		}
		return it;
	}

	void encode(){}
	void writeToDisk(){}
	void decode(){}
	void merge(){}
	void query(){}
	void display(){
		for(vector<postingList>::iterator it = ind.begin(); it != ind.end(); ++ it){
			cout << it->termID << ' ';
			for(vector<unsigned int>::iterator intit = (it->postings).begin(); intit != (it->postings).end(); ++intit){
				cout<< *intit << " ";
			}
			cout << endl;
		}
	}
};

vector<std::string> tokenize(string text) {
	std::string s = text;
	tokenizer<> tok(s);
	vector<std::string> vocabulary;
	for (tokenizer<>::iterator beg = tok.begin(); beg != tok.end(); ++beg) {
		vocabulary.push_back(*beg);
	}
	return vocabulary;
}

string readFile(string dir) {
	ifstream myFile;
	myFile.open(dir.c_str());
	string inputStream;
	string result = "";
	while(getline(myFile, inputStream)){
		result += inputStream;
	}
	myFile.close();
	return result;
}

int main(){
	unsigned int docId = 0;
	dictionary dictionary;
	index index;
	DIR *dir;
	struct dirent *ent;

	if ((dir = opendir("./test_html")) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL) {
			string dir = ent->d_name;
			string wordStream;
			if (dir != "." && dir != "..") {
				docId ++;
				//cout << dir << endl;
				wordStream = readFile("./test_html/" + dir);
				vector<string> voc = tokenize(wordStream);
				unsigned int pos = 0;

				for(vector<string>::iterator it = voc.begin(); it != voc.end(); it ++){
					int termId = dictionary.update(*it);
					index.update(termId, docId, pos);
					pos ++;
				}
				
				//buildPList(voc, docId, index);
				//buildPageTable(fileDir, docId, pageTable);
			}
		}
		//dictionary.display();
		index.display();
		/*
		closedir(dir);
		displayPList(postingList);
		writetoDisk(postingList, dic);
		displayDic(dic);
		*/
	}
	else {
		/* could not open directory */
		perror("Cannot open directory.");
		return EXIT_FAILURE;
	}


	return 0;
}