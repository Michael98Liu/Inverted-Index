#include <iostream>
#include <fstream>
using namespace std;

int main(){
    ifstream fin;
    fin.open("test");
    string line;
    while(getline(fin, line)){
        cout << line<< endl;
    }

    return 0;
}
