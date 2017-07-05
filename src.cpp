#include<iostream>
#include<boost/tokenizer.hpp>
#include<string>
#include<vector>
#include<list>
#include<map>
#include<fstream>
#include<bitset>
#include"dirent.h"
using namespace std;
using namespace boost;

string readFile(string dir) {
	ifstream myFile;
	myFile.open(dir);
	string inputStream;
	getline(myFile, inputStream);
	return inputStream;
}

list<string> tokenize(string text) {
	string s = text;
	tokenizer<> tok(s);
	list<string> vocabulary;
	for (tokenizer<>::iterator beg = tok.begin(); beg != tok.end(); ++beg) {
		vocabulary.push_back(*beg);
	}
	return vocabulary;
}

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

void encode(map < string, vector<unsigned int>> postingList, map<string, tuple<long, int>> &dic, ofstream &myfile) {
	long loc;
	for (map<string, vector<unsigned int>>::iterator it = postingList.begin(); it != postingList.end(); ++it) {
		char buffer[64] = { 0 };//64 is write-to-file-buffer size(block size)
		loc = myfile.tellp();
		map<string, tuple<long, int>>::iterator dicvoc;
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

void merge() {

}

void buildDic(list<string> voc, map<string, tuple<long, int>> &dic) {
	map<string, tuple<long,int>>::iterator findit;//map of {term} to {pointer in file(location), length of the block}
	for (list<string>::iterator it = voc.begin(); it != voc.end(); ++it) {
		findit = dic.find(*it);
		if (findit == dic.end()) {
			dic.insert(pair<string, tuple<long, int>>(*it, { 0,0 }));
		}
	}
}

void buildPageTable(string fileName, int docID, map<int, tuple<string, int>> &pageTable) {
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
	pageTable.insert(pair<int, tuple<string, int>>(docID, { fileName, size }));
}

void buildPList(list<string> voc, unsigned int docId, map < string, vector<unsigned int>> &postingList) {
	map < string, vector<unsigned int>>::iterator findit;
	for (list<string>::iterator it = voc.begin(); it != voc.end(); ++it) {
		findit = postingList.find(*it);
		if (findit == postingList.end()) {
			vector<unsigned int> posting;
			posting.push_back(docId);
			postingList.insert(pair<string, vector<unsigned int>>(*it, posting));
		}
		else {
			(findit->second).push_back(docId);
		}
	}
}

void writetoDisk(map < string, vector<unsigned int>> postingList, map<string, tuple<long,int>> &dic) {
	ofstream pList(".\\postinglists\\001", ofstream::binary);
	if (pList) encode(postingList, dic, pList); 
	else perror("Error opening file");
	pList.close();
}

vector<int> merge(vector<string> results) {
	vector<int> allP;
	return allP;
}

vector<int> query(string query, map<string, tuple<long, int>> &dic) {
	map<string, tuple<long, int>>::iterator dicvoc;
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

void displayVoc(list<string> voc) {
	for (list<string>::iterator it = voc.begin(); it != voc.end(); ++it) {
		cout << *it << endl;
	}
}

void displayDic(map<string, tuple<long,int>>dic) {
	cout << "Dictionary: " << endl;
	for (map<string, tuple<long, int>>::iterator it = dic.begin(); it != dic.end(); ++it) {
		cout << it->first << ' ' << get<0>(it->second) << ' ' <<  get<1>(it->second) << endl;
	}
}

void displayPList(map<string, vector<unsigned int>> pList) {
	cout << "Posting Lists: " << endl;
	for (map<string, vector<unsigned int>>::iterator it = pList.begin(); it != pList.end(); ++it) {
		cout << it->first << ' ';
		for (vector<unsigned int>::iterator vit = (it->second).begin(); vit != (it->second).end(); ++vit) {
			cout << *vit << ' ';
		}
		cout << endl;
	}
}

int main() {
	int docId = -2;
	map<string, tuple<long, int>> dic;
	map<string, vector<unsigned int>> postingList;
	map <int, tuple<string, int>> pageTable;
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(".\\files")) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL) {
			docId ++;
			string dir = ent->d_name;
			//printf("%s\n", dir);
			string wordStream;
			if (dir != "." && dir != "..") {
				string fileDir = "D:\\mystuff\\InvertedIndex\\files\\" + dir;
				wordStream = readFile(fileDir);
				list<string> voc = tokenize(wordStream);
				buildDic(voc, dic);
				buildPList(voc, docId, postingList);
				buildPageTable(fileDir, docId, pageTable);
			}
		}
		closedir(dir);
		displayPList(postingList);
		writetoDisk(postingList, dic);
		displayDic(dic);
	}
	else {
		/* could not open directory */
		perror("");
		return EXIT_FAILURE;
	}
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

	std::system("pause");
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
}*/
