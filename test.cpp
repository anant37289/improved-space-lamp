#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include "instructionmap.h"
using namespace std;
//cerates a string stream to tokenize by spaces
vector<string> tokenize(string instruction){
   vector <string> instr_seg;
    istringstream iss(instruction);
    string word;
    while(iss>>word){
        instr_seg.push_back(word);
    }
    return instr_seg;
}

string encode(string instruction,N::instr_map imap){
    vector<string> instr_seg=tokenize(instruction);
    vector<string> desc=imap.get_info(instr_seg[0]);
}
void assemble(string inpath,string opath){
N::instr_map imap1;
string fileContents;
ifstream inputFile(inpath);
ofstream outputFile(opath);
string instruction;
  while (getline(inputFile, instruction)) {
         outputFile <<encode(instruction,imap1)<<endl;
    }
}
int main() {
    assemble("example.txt","output.txt");
    string s="test string";
}
