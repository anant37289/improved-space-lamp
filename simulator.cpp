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

class controller{
    public:
    map<string,bool> cw_map;
    controller(string opcode){
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
    controller(){

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
//DM
map<int,string> IM;
//IM
int GPR[32];
int ins[32];
//GPR
class ALU{
    public:
    int compute(int ip1,int ip2,string ALU_sel){
        int select=stoi(ALU_sel,nullptr,2);
        //multiplexing on select
        switch (select){
            case 0:return ip1+ip2;
            case 1:return ip1-ip2;  
            case 2:return ip1|ip2;
            case 3:return ip1&ip2;
        }
    }
}alu;

//the pipeline register set
struct PC_register_set{
    public:
    int pc;
    bool valid;
}PC;
struct IFID_register_set{
    public:
    string IR;
    int DPC;
    bool valid;//default valid=false
}ifid;

typedef struct func_bits{
    int DPC ;
    string opcode;
    string func3;
    string func7;  

}func;

struct  IDEX_register_set{
    int DPC;
    string instruction;
    int DPC;
    int immf;
    controller cw;
    int rs1;
    int rs2;
    func func_b;
    int rdl;
    bool valid;
}idex;

struct EXMO_register_set{
    int DPC;
    controller cw;
    string instruction;
    int ALU_out;
    int rs2;
    int rdl;
    bool valid;
}exmo;

struct MOWB_register_set{
    int DPC;
    string instruction;
    controller cw;
    int ld_out;
    int alu_out;
    int rdl;
    bool valid;
}mowb;
// global NPC
int NPC;
int TPC;
string s(31,'0');


void fetch(){
if (!PC.valid){return;}
    string instruction=IM[PC.pc];

    if(instruction==s){PC.valid=false;}
    ifid.IR=instruction;
    ifid.DPC=PC.pc;
    
    ifid.valid=true;
    return;
}

void decode(){
    if(!ifid.valid){return;}
    string instruction=ifid.IR;
    if(instruction==s){ifid.valid=false;}
    int PC_decode=ifid.DPC;
    //immediate generator
    string immil=instruction.substr(0,12);//imm for immediate & load 
    string imms=instruction.substr(0,7)+instruction.substr(20,5);
    string immb=instruction.substr(0,1)+instruction.substr(24,1)+instruction.substr(1,6)+instruction.substr(20,4);
    //

    int rsl1=stir(instruction.substr(12,5));
    int rsl2=stir(instruction.substr(7,5));
    int rd=stir(instruction.substr(20,5));
    string opcode=instruction.substr(25,7);

    //making control word
    controller cw(opcode);
    
    //register read
    //decide what type of instruction it is and put the locks accordingly

    int rs1,rs2;
    if(cw.cw_map["regread"] && ins[rsl1]==-1){rs1=GPR[rsl1];}
    else{
        //stall
    }
    if(cw.cw_map["regread"] &&  (opcode=="0110011"|| opcode=="0100011") && ins[rsl2]==-1)
    {rs2=GPR[rsl2];}
    else{
        //stall
    }
    //int immf;//!cw.cw_map["ALUsrc"](checks if it is R type)
    //assigning imm in idex
    if(cw.cw_map["memwrite"]){
        idex.immf=stii(imms);
    }
    else if(cw.cw_map["branch"]){
        idex.immf=stii(immb);
    }
    else{
        idex.immf=stii(immil);
    }
    //decoding for func3 and func7
    string func3=instruction.substr(17,3);
    string func7=instruction.substr(0,7);
    //
    idex.instruction=instruction;
    idex.DPC=PC_decode;
    idex.cw=cw;
    idex.rs1=rs1;
    idex.rs2=rs2;
    idex.func_b.func3=func3;
    idex.func_b.func7=func7;
    idex.func_b.opcode=opcode;
    idex.rdl=rd;
    idex.valid=true;
    return;
}
void execute(){
    if(!idex.valid){return;}
    string instruction=idex.instruction;
    if(instruction==s){idex.valid=false;}
    int PC_execute=idex.DPC;
    int immf=idex.immf;
    controller cw=idex.cw;
    int rs1=idex.rs1;
    int rs2=idex.rs2;
    func_bits func_b=idex.func_b;
    string func3=func_b.func3;
    string func7=func_b.func7;
    string opcode=func_b.opcode;
    int rd=idex.rdl;

    //generating ALU control
    string aluselect=ALU_control(func3,func7,opcode);
    //

    //for execution
    int input_data1=rs1;
    int input_data2;
    if(cw.cw_map["ALUsrc"]){
        input_data2=immf;
    }else{
    input_data2=rs2;
    }
     
    int ALU_result=alu.compute(input_data1,input_data2,aluselect);
    bool zero_flag=(rs1==rs2);
    bool lt_flag=(rs1<rs2);
    bool gt_flag=(rs1>rs2);
   

    int BPC= PC_execute+immf*4;
    // int TPC;
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
    //PC.pc=TPC;//here the hazard logic and flushing the pipeline will come
exmo.DPC=idex.DPC;
exmo.instruction=instruction;
exmo.cw=cw;
exmo.ALU_out=ALU_result;
exmo.rdl=rd;
exmo.rs2=rs2;
exmo.valid=true;
return;
}
void memory(){
    if(!exmo.valid){return;}
    string instruction=exmo.instruction;
    if(instruction==s){exmo.valid=false;}
    controller cw=exmo.cw;
    int ALU_result=exmo.ALU_out;
    int rd=exmo.rdl;
    int rs2=exmo.rs2;
    int ldresult;
   if(cw.cw_map["memread"]){
        ldresult=dm.DataMem[ALU_result];
    }
    if(cw.cw_map["memwrite"]){
        dm.write(ALU_result,rs2);
    }
mowb.DPC=exmo.DPC;
mowb.instruction=instruction;
mowb.alu_out=exmo.ALU_out;
mowb.cw=exmo.cw;
mowb.ld_out=ldresult;
mowb.rdl=rd;
mowb.valid=true;
    }
void wb(){
    if(!mowb.valid){return;}
    string instruction = mowb.instruction;
    if(instruction==s){mowb.valid=false;}
    int ALU_result=mowb.alu_out;
    int ldresult=mowb.ld_out;
    controller cw=mowb.cw;
    int rd=mowb.rdl;

      if(cw.cw_map["regwrite"]){
        if(cw.cw_map["memreg"]){
            GPR[rd]=ldresult;
        }else{
             GPR[rd]=ALU_result;
        }
    }
    }
void make_IM_GPR_INS(){
for(int i=0;i<32;i++){GPR[i]=0;ins[i]=-1;}
string instruction;
ifstream inputfile("machinecode.txt");
int PC=0;
while(getline(inputfile,instruction)){
IM[PC]=instruction;
PC=PC+4;
}
string s(31,'0');
s=s+"1";
IM[PC]=s;
//return PC_exit;
}

int main(){
    s=s+"1";
make_IM_GPR_INS();
//load the pipeline
PC.valid=true;
PC.pc=0;
int CC=0;
//int CC_after_branch_fetch=0;
while(PC.valid||ifid.valid||idex.valid||exmo.valid||mowb.valid){//just fetch the last instruction
    //check precoondition
    CC+=1;
    cout<<CC<<endl;
    //operant forwarding should also happen here
    NPC=PC.pc+4;
    wb();
    memory();
    execute();
    decode();
    fetch();
    if(CC<3){
        PC.pc=NPC;
    }
    else{
        PC.pc=TPC;
    }
}
}

