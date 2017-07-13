#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>
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
    ifstream infile;
    infile.open("00");
    if(infile.is_open()){
        string html((std::istreambuf_iterator<char>(infile)), istreambuf_iterator<char>());
        CDocument doc;
        doc.parse(html.c_str());

        CSelection c = doc.find("html");
        //cout << html << endl;
        cout << c.nodeAt(0).text() << std::endl; // some link
        //deletePage(infile);)
    }else{
        cerr << "File cannot be opened." << endl;
    }
}




int main(int argc, char * argv[])
{
    parseFile();

    return 0;
}
