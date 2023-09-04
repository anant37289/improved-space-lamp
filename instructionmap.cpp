#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <sstream>
#include "instructionmap.h"
using namespace N;
using namespace std;
//takes in the description to give a vector of info
vector<string> tokeniz(string instr_desc){
    vector<string> desc;
    stringstream lineStream(instr_desc);
    string cell;
    while(getline(lineStream,cell,',')){
        desc.push_back(cell);
    }
    return desc;
}
//some desc

    
instr_map::instr_map(){
string inpath="instruction.csv";
ifstream inputFile(inpath);
string instruction;
while (getline(inputFile, instruction)) {
        int first_comma=instruction.find(',');
        instr[instruction.substr(0,first_comma)]=tokeniz(instruction.substr(first_comma+1));
}
}
vector<string> instr_map::get_info(string operation){
    return instr[operation];
}

