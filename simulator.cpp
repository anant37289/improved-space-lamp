#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
using namespace std;

//string to integer immediate
int stii(string bin){
//string bin="100000000000";
char signbit=bin[0];
string mag=bin.substr(1);
int imm;
if(signbit=='1'){
    //7FF is the 11 bit all 1 integer
    int complement_mag=stoi(mag,nullptr,2);
    int complement_1s=complement_mag^0x7FF;
    int complement_2s=complement_1s+1;
    imm=-complement_2s;
    //cout<<imm<<endl;
}
else{
    int complement_mag=stoi(mag,nullptr,2);
    imm=complement_mag;
}
    return imm;
}

//string to register
int stir(string reg){
    return stoi(reg,nullptr,2);
}

string reverse_string(string unrev){
    string rev;
    for (auto it = unrev.rbegin(); it != unrev.rend(); ++it) {
        rev += *it;
    }
    return rev;
}

string substr_rev(int start,int length,string rev_instr){
    string unrev=rev_instr.substr(start,length);
    string rev=reverse_string(unrev);
    return rev;
}
class controlword{
    public:
    map<string,bool> cw_map;
    controlword(string opcode){
        bool op0=stoi(opcode.substr(0,1));
        bool op1=stoi(opcode.substr(1,1));
        bool op2=stoi(opcode.substr(2,1));
        cw_map["ALUsrc"]=!op1||!op0&&op1&&!op2;
        cw_map["mem_Reg"]=op0;
        cw_map["reg_write"]=!op1||op2;
        cw_map["mem_write"]=!op0&&op1&&!op2;
        cw_map["Branch"]=op0&&op1;
        cw_map["aluop1"]=op2;
        cw_map["aluop2"]=op0&&op1;
        cw_map["regread"]=1;
        //for testing
        //cout<<cw_map["ALUsrc"]<<" "<<cw_map["mem_Reg"]<<" "<<cw_map["reg_write"];
    }
};

string ALU_control(string func3,string func7,bool aluop1,bool aluop2){
    bool func7_30=stoi(func7.substr(1,1));
    bool func3_0=stoi(func3.substr(0,1));
    bool func3_1=stoi(func3.substr(1,1));
    bool func3_2=stoi(func3.substr(2,1));
    string ALUC="0";
    
}
int main(){
int GPR[32];
string instruction;
ifstream inputfile("machinecode.txt");
int PC=0;
//instruction=IM[PC]
while(getline(inputfile,instruction)){
    PC=PC+4;
    string reverse_instruction=reverse_string(instruction);
    //using different instruction indexing
    //since it has been revesed we reverse it again
    int rsl1=stir(substr_rev(15,5,reverse_instruction));
    int rsl2=stir(substr_rev(20,5,reverse_instruction));
    int rd=stir(substr_rev(7,5,reverse_instruction));
    string opcode=substr_rev(0,7,reverse_instruction);
    controlword cw(opcode);
    // Reg access
    if(cw.cw_map["regread"]){int rs1=GPR[rsl1];}
    if(cw.cw_map["regread"]){int rs2=GPR[rsl2];}
    string func3=substr_rev(12,3,reverse_instruction);
    string func7=substr_rev(25,7,reverse_instruction);
    //cout<<PC<<" "<<opcode<<endl;
    string alucontrol=ALU_control(func3,func7,cw.cw_map["aluop1"],cw.cw_map["aluop2"]);

}
}
