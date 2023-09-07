#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <bitset>
#include<list>
#include<map>
using namespace std;


class instr_map{
map<string,string> instr;
public:
instr_map()
{string inpath="instruction.csv";
ifstream inputFile(inpath);
string instruction;
while (getline(inputFile, instruction)) {
        int first_comma=instruction.find(',');
        instr[instruction.substr(0,first_comma)]=(instruction.substr(first_comma+1));
}
}
string get_info(string operation){
    return instr[operation];
}
}imap;

class reg_map{
   map<string,string> register_mapping;
   public:
   reg_map(){
    string inpath="register.csv";
    ifstream inputFile(inpath);
    string regdef;
    while (getline(inputFile, regdef)) {
            int first_comma=regdef.find(',');
            register_mapping[regdef.substr(0,first_comma)]=(regdef.substr(first_comma+1));
}
}
   string get_binary(string reg){
    return register_mapping[reg];
}
}rmap;

class symtab{
public:
map<string,int> symbol_map;

int get_label_line(string label){
    return symbol_map[label];
}
}symtab1;

class encoder{
public:
string rencoding(vector<string> desc_tokens,vector<string> instr_seg){
    return desc_tokens[3]+rmap.get_binary(instr_seg[3])+rmap.get_binary(instr_seg[2])+desc_tokens[2]+rmap.get_binary(instr_seg[1])+desc_tokens[1];
}
string iencoding(vector<string> desc_tokens,vector<string> instr_seg){
    int imme=stoi(instr_seg[3]);
    string imm=bitset<12>(imme).to_string();
    return imm+rmap.get_binary(instr_seg[2])+desc_tokens[2]+rmap.get_binary(instr_seg[1])+desc_tokens[1];
}

string sencoding(vector<string> desc_tokens,vector<string> instr_seg){
   string imm=bitset<12>(stoi(instr_seg[2])).to_string();
   return imm.substr(0,7)+rmap.get_binary(instr_seg[1])+rmap.get_binary(instr_seg[3])+desc_tokens[2]+imm.substr(7)+desc_tokens[1];
}
string bencoding(vector<string> desc_tokens,vector<string> instr_seg,int instruction_number){
    string label=instr_seg[3];
    int label_def=symtab1.symbol_map[label];
    int immi=(label_def-instruction_number);
    string imm=bitset<12>(immi).to_string();
    cout<<immi<<" "<<imm<<endl;
    return imm[0]+imm.substr(2,6)+rmap.get_binary(instr_seg[2])+rmap.get_binary(instr_seg[1])+desc_tokens[2]+imm.substr(8)+imm[1]+desc_tokens[1];
}   
//jump and u type maybe later
}encoder1;

//helper functions
//check if the instruction is label or instruction
bool is_label(string instruction){
    return instruction.back()==':';
}

void make_symbol_table(string inpath){
ifstream inputFile(inpath);
string instruction;
int count=0;
list <string> labels_need_to_add;
cout<<"yes"<<endl;
  while (getline(inputFile, instruction)) {
    cout<<"yes"<<endl;
    cout<<count<<endl;
        if(is_label(instruction)){
            
            instruction.pop_back();
            labels_need_to_add.push_back(instruction);
            while(getline(inputFile, instruction)){
                if(is_label(instruction)){
                   labels_need_to_add.push_back(instruction); 
                }
                else{
                    count++;
                    while(!labels_need_to_add.empty()){
                        string label=labels_need_to_add.front();
                        labels_need_to_add.pop_front();
                        symtab1.symbol_map[label]=count;
                    }
                    break;
                }
            }
        }else{
            count++;
        }
    }
}

string encoding(string desc,vector<string> instr_seg,int instruction_count){
    //tokenize desc
    vector <string> desc_tokens;
    stringstream ss(desc);
    string token;
    while (getline(ss, token, ',')) {
        desc_tokens.push_back(token);
    }
    if(desc[0]=='R'){
        return encoder1.rencoding(desc_tokens,instr_seg);
    }
    else if(desc[0]=='I'){
        return encoder1.iencoding(desc_tokens,instr_seg);
    }
    else if(desc[0]=='S'){
        return encoder1.sencoding(desc_tokens,instr_seg);
    }
    else if(desc[0]='B'){
        //patch the labels->need line number
        return encoder1.bencoding(desc_tokens,instr_seg,instruction_count);
    }

}

vector<string> tokenize(string instruction){
   vector <string> instr_seg;
    int first_space=instruction.find(' ');
    instr_seg.push_back(instruction.substr(0,first_space));
    string operant=instruction.substr(first_space+1);
    stringstream ss(operant);
    string token;
    while (getline(ss, token, ',')) {
        instr_seg.push_back(token);
    }
    return instr_seg;
}

string encode(string instruction,int instruction_count){
    vector<string> instr_seg=tokenize(instruction);
    //cout<<instr_seg[0]<<" "<<typeid(instr_seg[0]).name()<<endl;
    string desc=imap.get_info(instr_seg[0]);
    return encoding(desc,instr_seg,instruction_count);
}

void assemble(string inpath,string opath){
make_symbol_table(inpath);
//string fileContents;
ifstream inputFile(inpath);
ofstream outputFile(opath);
string instruction;
int instruction_count=0;
  while (getline(inputFile, instruction)) {
    if(!is_label(instruction)){
        instruction_count++;
        outputFile <<encode(instruction,instruction_count)<<endl;
    }    
    }
}
int main() {
   assemble("example.txt","machinecode.txt");
}
