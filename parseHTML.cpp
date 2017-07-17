#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>
#include <dirent.h>
#include <string.h>
#include "gq/Document.h"
#include "gq/Node.h"
using namespace std;

bool is_empty(ifstream& pFile){
    return pFile.peek() == ifstream::traits_type::eof();
}

// void deletePage(ifstream &fin){
//     string line;
//     while (getline(fin,line)){
//         line.replace(line.find(deleteline),deleteline.length(),"");
//     }
// }

void parseFile(){
    string indir = "./seperate_html/";
    string outdir = "./parsed_html/";
    ifstream infile;
    ofstream fout;
    ofstream urlTable;
    ofstream pageTable;
    pageTable.open("pageTable");
    urlTable.open("urlTable");

    int count = 0;
    int startPos = 0;

    DIR *pDIR;
    struct dirent *entry;
    if( pDIR=opendir(indir.c_str()) ){
        while(entry = readdir(pDIR)){
            if( strcmp(entry->d_name,".") != 0 && strcmp(entry->d_name,"..") != 0) {
                cout << "Processing " << entry->d_name << " ...\n";
                infile.open((indir + entry->d_name).c_str());
                if(infile.is_open()){
                    fout.open((outdir+to_string(count)).c_str());
                    string html((std::istreambuf_iterator<char>(infile)), istreambuf_iterator<char>());
                    CDocument doc;
                    doc.parse(html.c_str());

                    CSelection c = doc.find("html");
                    //cout << html << endl;
                    fout << c.nodeAt(0).text() << std::endl; // some link
                    //deletePage(infile);)
                    fout.close();
                    CSelection u = doc.find("DOCHDR");
                    string utext = u.nodeAt(2).text();
                    int find1 = utext.find("http");
                    int find2 = utext.find("HTTP");
                    string url = utext.substr(find1, find2 - 2);
                    startPos = urlTable.tellp();
                    urlTable << url;
                    pageTable << startPos << endl;
                    
                }else{
                    cerr << "File cannot be opened." << endl;
                }
                infile.close();
                count ++;
            }
        }
    }
    pageTable.close();
    urlTable.close();
}




int main(int argc, char * argv[])
{
    int start_s = clock();
    parseFile();
    int stop_s = clock();
    cout << "time: " << (stop_s-start_s)/double(CLOCKS_PER_SEC) << endl;

    return 0;
}
