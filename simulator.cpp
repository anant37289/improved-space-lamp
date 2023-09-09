#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
using namespace std;
//ALU select of 2 bits for ease
//design decision to get rid of ALU op and give opcode to ALUcontrol
//choice of int for its intrinsic 4 bytes for register
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

// string reverse_string(string unrev){
//     string rev;
//     for (auto it = unrev.rbegin(); it != unrev.rend(); ++it) {
//         rev += *it;
//     }
//     return rev;
// }

// //returns the substring reversed
// string substr_rev(int start,int length,string rev_instr){
//     string unrev=rev_instr.substr(start,length);
//     string rev=reverse_string(unrev);
//     return rev;
// }
class controlword{
    public:
    map<string,bool> cw_map;
    controlword(string opcode){
        //first 3 bits of opcode
        bool op0=stoi(opcode.substr(0,1));
        bool op1=stoi(opcode.substr(1,1));
        bool op2=stoi(opcode.substr(2,1));//opcode_6,5,4 respectively
        cw_map["ALUsrc"]=!op1||!op0&&op1&&!op2;//ok
        cw_map["memreg"]=!op0&&!op1&&!op2;
        cw_map["regwrite"]=!op1||op2;
        cw_map["memwrite"]=!op0&&op1&&!op2;
        cw_map["memread"]=!op0&&!op1&&!op2;
        cw_map["branch"]=op0&&op1;
        // cw_map["aluop1"]=op2;
        // cw_map["aluop2"]=op0&&op1;
        cw_map["regread"]=1;
        //only the things completely dermined from opcode

        //for testing
        //cout<<cw_map["ALUsrc"]<<" "<<cw_map["mem_Reg"]<<" "<<cw_map["reg_write"];
    }
};

string ALU_control(string func3,string func7,string opcode){
    //getiing the relevent bits
    bool func7_30=stoi(func7.substr(1,1));//the 30th bit
    bool func3_0=stoi(func3.substr(0,1));
    bool func3_1=stoi(func3.substr(1,1));
    bool func3_2=stoi(func3.substr(2,1));//all the func3 bits
    bool op0=stoi(opcode.substr(0,1));//useless same for all required
    bool op1=stoi(opcode.substr(1,1));
    bool op2=stoi(opcode.substr(2,1));

    int ALUC_1=func3_0;
    int ALUC_2=(func7_30||func3_2)&&op1&&op2;
    return to_string(ALUC_1)+to_string(ALUC_2);
    
}

class DM{
    public:
    map<int,int> DataMem;
    DM(){
        ifstream dat_file("DM.csv");
        string dat_line;
        while(getline(dat_file,dat_line)){
            int comma=dat_line.find(',');
            DataMem[stoi(dat_line.substr(0,comma))]=stoi(dat_line.substr(comma+1));
        }
    }
        void write(int address,int data){
        ofstream dat_file1("DM.csv",ios::app);
        dat_file1<<to_string(address)+","+to_string(data)<<endl;
    }
}dm;

class ALU{
    public:
    int compute(int ip1,int ip2,string ALU_sel){
        int select=stoi(ALU_sel,nullptr,2);
        //multiplexing on sel
        switch (select){
            case 0:return ip1+ip2;
            case 2:return ip1|ip2;
            case 3:return ip1&ip2;
            case 1:return ip1-ip2;  
        }
    }
}alu;
int main(){
int GPR[32];
for(int i=0;i<32;i++){GPR[i]=0;}
string instruction;
ifstream inputfile("machinecode.txt");
int PC_proxy=0;
map<int,string> IM;
while(getline(inputfile,instruction)){
IM[PC_proxy]=instruction;
PC_proxy=PC_proxy+4;
}
cout<<PC_proxy<<endl;//finally holds the line next to last
int PC=0;
int NPC;
//
// while(getline(imem,instruction)){
    while(PC!=PC_proxy){
    instruction=IM[PC];
    NPC=PC+4;
    //using different instruction indexing

    string immil=instruction.substr(0,12);//imm for load store
    string imms=instruction.substr(0,7)+instruction.substr(20,5);
    string immb=instruction.substr(0,1)+instruction.substr(24,1)+instruction.substr(1,6)+instruction.substr(20,4);
    //debugging purpose
    cout<<instruction<<endl;
    cout<<instruction.substr(0,1)<<" "<<instruction.substr(24,1)<<" "<<instruction.substr(1,6)<<" "<<instruction.substr(20,4)<<endl;
    int rsl1=stir(instruction.substr(12,5));
    int rsl2=stir(instruction.substr(7,5));
    int rd=stir(instruction.substr(20,5));
    string opcode=instruction.substr(25,7);
    controlword cw(opcode);
    // Reg access
    //test of cw
    cout<<cw.cw_map["ALUsrc"]<<" "<<cw.cw_map["memreg"]<<" "<<cw.cw_map["regwrite"]<<" "<<cw.cw_map["memwrite"]<<" "<<cw.cw_map["branch"]<<endl;
    int rs1,rs2;
    if(cw.cw_map["regread"]){rs1=GPR[rsl1];}
    if(cw.cw_map["regread"]){rs2=GPR[rsl2];}
    int immi;
    if(cw.cw_map["memwrite"]){
        immi=stii(imms);
    }
    else if(cw.cw_map["branch"]){
        immi=stii(immb);
    }
    else{
        immi=stii(immil);
    }
  
    string func3=instruction.substr(17,3);
    string func7=instruction.substr(0,7);
    //cout<<PC<<" "<<opcode<<" "<<func3<<" "<<func7<<endl;
    string aluselect=ALU_control(func3,func7,opcode);
    //Execute
    cout<<aluselect<<endl;
    int input_data1=rs1;
    int input_data2;
    if(cw.cw_map["ALUsrc"]){
        input_data2=immi;
    }else{
    input_data2=rs2;
    }
     
    int ALU_result=alu.compute(input_data1,input_data2,aluselect);
    bool zero_flag=(rs1==rs2);
    bool lt_flag=(rs1<rs2);
    bool gt_flag=(rs1>rs2);
    int ldresult;
    if(cw.cw_map["memread"]){
        ldresult=dm.DataMem[ALU_result];
    }
    if(cw.cw_map["memwrite"]){
        dm.write(ALU_result,rs2);
    }
    int BPC=PC+immi*4;
    int TPC;
    if(cw.cw_map["branch"]){
        switch (stir(func3)){
            case 0://beq
            if(zero_flag){TPC=BPC;}
            else{TPC=NPC;}
            break;
            case 5:
            if(zero_flag||gt_flag){TPC=BPC;}
            else{TPC=NPC;}
            break;
            case 7:
            if(lt_flag){TPC=BPC;}
            else{TPC=NPC;}
            break;
        }
    }else{
        TPC=NPC;
    }
    PC=TPC;
    if(cw.cw_map["regwrite"]){
        if(cw.cw_map["memreg"]){
            GPR[rd]=ldresult;
        }else{
             GPR[rd]=ALU_result;
        }
    }
    cout<<rd<<endl;
    cout<<"s2 "<<GPR[18]<<endl;
    //}
    }
}
// int main(){
//     cout<<dm.DataMem[2]<<endl;
//     dm.write(3,4068);
// }
