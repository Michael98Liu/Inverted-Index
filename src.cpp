#include<iostream>
#include<boost/tokenizer.hpp>
#include<string>
#include<vector>
#include<fstream>
#include<bitset>
#include"dirent.h"
using namespace std;
using namespace boost;

class dictionary{
private:
	struct vocabulary{
		string term;
		long location; //start location in the posting list
		int length; //length of the block in the posting list
	};

	vector<vocabulary> dict;

public:
	void update(vector<string> voc){
		for(vector<string>::iterator it = voc.begin(); it != voc.end(); ++it){
			if(find(*it)) continue;
			else add(*it);
		}
	}

	vocabulary *find(string term){ //return a vocabulary struct
		for(vector<vocabulary>::iterator it = dict.begin(); it != dict.end(); ++it){
			if(it->term == term){
				vocabulary* voc = new vocabulary;
				voc->term = term;
				voc->location = it->location;
				voc->length = it->length;
				return voc;
			}
		}
		return nullptr;
	}

	void add(string term){
		dict.push_back({term, 0, 0});
	}

	void display(){
		for(vector<vocabulary>::iterator it = dict.begin(); it != dict.end(); ++it){
			cout << it->term << ' ' << it->location << ' ' << it->length << endl;
		}
	}

	void update(string term, long loc, int len){
		vocabulary* dicvoc = find(term);
		dicvoc->location = loc;
		dicvoc->length = len;
	}
};

class index{
private:
	struct postingList{
		string term;
		vector<unsigned int> postings;
	};

	vector<postingList> ind;

public:
	void update(vector<string> voc, unsigned int docID){
		for (vector<string>::iterator it = voc.begin(); it != voc.end(); ++it) {
			postingList* findterm = find(*it);
			if (findterm) {
				(findterm->postings).push_back(docID);
				// cout << findterm->term << endl;
				// for(vector<unsigned int>::iterator it2 = (findterm->postings).begin(); it2 != (findterm->postings).end(); ++it2){
				// 	cout << *it2 << ' ';
				// }
				// cout << endl;
			}
			else {
				postingList *posting = new postingList;
				posting->term = *it;
				(posting->postings).push_back(docID);
				ind.push_back(*posting);
			}
		}
	}

	postingList *find(string term){
		for(vector<postingList>::iterator it = ind.begin(); it != ind.end(); ++ it){
			if(it->term == term){
				postingList *pos = new postingList;
				pos->term = term;
				pos->postings = &it->postings;
				return pos;
			}
		}
		return nullptr;
	}

	void encode(){}
	void writeToDisk(){}
	void decode(){}
	void merge(){}
	void query(){}
	void display(){
		for(vector<postingList>::iterator it = ind.begin(); it != ind.end(); ++ it){
			cout << it->term << ' ';
			for(vector<unsigned int>::iterator intit = (it->postings).begin(); intit != (it->postings).end(); ++intit){
				cout<< *intit << " ";
			}
			cout << endl;
		}
	}
};

class pageTable{
public:

private:
	struct page{
		int docID;
		string url;
		int length;
	};

	vector<page> table;
};

string readFile(string dir) {
	ifstream myFile;
	myFile.open(dir.c_str());
	string inputStream;
	getline(myFile, inputStream);
	return inputStream;
}

vector<string> tokenize(string text) {
	string s = text;
	tokenizer<> tok(s);
	vector<string> vocabulary;
	for (tokenizer<>::iterator beg = tok.begin(); beg != tok.end(); ++beg) {
		vocabulary.push_back(*beg);
	}
	return vocabulary;
}

/*
string VBEncode(vector<unsigned int> uncomp) {
	string buffer;
	string mybits;
	for (vector<unsigned int>::iterator it = uncomp.begin(); it != uncomp.end(); ++it) {
		mybits = "";
		while ((*it) > 127) {
			mybits += '0';
			mybits += string(7, '1');
			(*it) -= 127;
		}
		mybits += '1';
		mybits += std::bitset<7>(*it).to_string();
		buffer += mybits;
	}
	return buffer;
}

void encode(std::map< string, vector<unsigned int> > postingList, std::map<string, tuple<long, int> > &dic, ofstream &myfile) {
	long loc;
	for (map<string, vector<unsigned int> >::iterator it = postingList.begin(); it != postingList.end(); ++it) {
		char buffer[64] = { 0 };//64 is write-to-file-buffer size(block size)
		loc = myfile.tellp();
		map<string, tuple<long, int> >::iterator dicvoc;
		dicvoc = dic.find(it->first);
		get<0>(dicvoc->second) = loc; //update dictionary
		vector<unsigned int> posting = it->second;
		int count = 0;
		string output = VBEncode(posting);
		while (output.length() > 64) {
			strcpy(buffer, output.substr(0, 64).c_str());
			output.erase(0, 64);
			myfile.write(buffer, 64);
			count++;
		}
		strcpy(buffer, output.c_str());
		cout << endl;
		myfile.write(buffer, 64);
		count++;
		get<1>(dicvoc->second) = count;
	}
}

vector<int> decode(char* block, int size) {
	vector<int> postings;
	string integer;
	int carry = 0;//for number greater then 127
	int i = 0;
	while (i < size & block[i] != NULL ) {
		integer = "";
		while (block[i] == '0') {
			carry += 127;
			i += 8;
		}
		for (int k = i + 1; k < i + 8; k++) {
			integer += block[k];
		}
		char* ptr = new char[integer.length()];
		strcpy(ptr, integer.c_str());
		long posting = strtol(ptr, &ptr, 2);
		postings.push_back(posting + carry);
		i += 8;
	}
	return postings;
}

void buildPageTable(string fileName, int docID, vector<page> &pageTable) {
	FILE * pFile;
	long size;
	const char* fileDir = fileName.c_str();

	pFile = fopen(fileDir, "rb");
	if (pFile == NULL) perror("Error opening file");
	else {
		fseek(pFile, 0, SEEK_END);   // non-portable
		size = ftell(pFile);
		fclose(pFile);
	}
	pageTable.insert(pair<int, tuple<string, int> >(docID, { fileName, size }));
}

void buildPList(list<string> voc, unsigned int docId, vector<postingList> &index) {

}

void writetoDisk(map < string, vector<unsigned int>> postingList, map<string, tuple<long,int> > &dic) {
	ofstream pList(".\\postinglists\\001", ofstream::binary);
	if (pList) encode(postingList, dic, pList);
	else perror("Error opening file");
	pList.close();
}

vector<int> merge(vector<string> results) {
	vector<int> allP;
	return allP;
}

vector<int> query(string query, map<string, tuple<long, int> > &dic) {
	map<string, tuple<long, int> >::iterator dicvoc;
	list<string> stream;
	vector<int> allPosting;
	stream = tokenize(query);
	for (list<string>::iterator it = stream.begin(); it != stream.end(); ++it) {
		dicvoc = dic.find(*it);
		if (dicvoc != dic.end()) {
			int size = get<1>(dicvoc->second) * 64;
			char memblock[64] = { 0 };
			ifstream file(".\\postinglists\\001", ifstream::binary);
			if (file) {
				int pos = get<0>(dicvoc->second);
				file.seekg(pos);//set seekg to the beginning of the block, size is the number of blocks
				file.read(memblock, size);
				file.close();
			}
			allPosting = decode(memblock, size);
		}
	}
	return allPosting;
}
*/

int main() {
	unsigned int docId = 0;
	dictionary dictionary;
	index index;
	pageTable pageTable;
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir("./files")) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL) {
			string dir = ent->d_name;
			string wordStream;
			if (dir != "." && dir != "..") {
				docId ++;
				cout << dir << endl;
				string fileDir = "./files/" + dir;
				wordStream = readFile(fileDir);
				vector<string> voc = tokenize(wordStream);
				dictionary.update(voc);
				index.update(voc, docId);
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
		perror("a");
		return EXIT_FAILURE;
	}
	/*
	cout << "Please enter your query:" << endl;
	string in;

	while (getline(cin, in)) {
		vector<int> docs;
		map<int, tuple<string, int>>::iterator docName;
		docs = query(in, dic);
		for (vector<int>::iterator it = docs.begin(); it != docs.end(); ++it) {
			docName = pageTable.find(*it);
			cout << get<0>(docName->second) << ' ' << get<1>(docName->second) << endl;
		}
		cout << "Please enter your query:" << endl;
	}
	*/
}

/*FILE * pList;
pList = fopen("D:\\mystuff\\InvertedIndex\\postinglists\\001", "r");
long loc = get<0>(dicvoc->second);
long len = get<1>(dicvoc->second);
fseek(pList, loc, SEEK_SET);
fgets(result, len, pList);
string res(result);*/
/*int i = 0;
for (tokenizer<>::iterator beg = tok.begin(); beg != tok.end(); ++beg) {
int intTok = stoi(*beg);
if (allPosting.size() != 0) {
while (i < allPosting.size() - 1 && intTok > allPosting.at(i)) {
i++;
}
if (intTok < allPosting.at(i) || i == allPosting.size() - 1)
allPosting.push_back(intTok);
}
else {
allPosting.push_back(stoi(*beg));
}
}

void buildDic(list<string> voc, vector<vocabulary> &dic) {
	vector<vocabulary>::iterator findit;//map of {term} to {pointer in file(location), length of the block}
	for (list<string>::iterator it = voc.begin(); it != voc.end(); ++it) {
		findit = dic.find(*it);
		if (findit == dic.end()) {
			dic.push_back({*it, 0, 0 });
		}
	}
}*/
