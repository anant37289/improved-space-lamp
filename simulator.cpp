#include <bits/stdc++.h>
using namespace std;
ofstream globalOutputStream;
enum type{read,write};
class block{
    public:
    int block_elements[16];
};
class req{
    public:
    int addr;
    type t;
    int tag(){
        return addr>>12;
    }
    int index(){
        return addr>>6 & 0x3F;//0x3F==0b11 1111
    }
    int offset(){
        return addr>>2 & 0xF;//i have made the memory word addressible so the addr should be right shifted by 2
    }
};
class cpureq:public req{
    public:
    int data;
};
class memreq:public req{
    public:
    block data;
    int tag(){
        return addr>>6;
    }
};
class resp{
    public:
    int addr;
    type t;
    int tag(){
        return addr>>12;
    }
    int index(){
        return addr>>6 & 0x3F;
    }
    int offset(){
        return addr>>2& 0xF;
    }
};
class cpuresp:public resp{
    public:
    int data;
};
class memresp:public resp{
    public:
    block data;
    int tag(){
        return addr>>6;
    }
};

class MM{
    public:
    block mm[4096];//actual size with int address should be 2^28(no need for that much space in simulator)
    memresp service(memreq MEMreq){
        this_thread::sleep_for(std::chrono::seconds(1));
        memresp MEMresp;
        if(MEMreq.t==read){
            MEMresp.addr=MEMreq.addr;
            MEMresp.data=mm[MEMreq.tag()];
            MEMresp.t=read;//against a read or a write
            return MEMresp;
        }
        if(MEMreq.t==write){
            mm[MEMreq.tag()]=MEMreq.data;
        } 
    }
}mem;
enum state{I,V,MP,M};
class info_block{
    //stoes the block info in a (S,T,B)
    public:
    block blck;
    int tag;//tag
    state s;
    int time_use;
};
class SET{
    public:
    info_block blocks[4];//way=4
  
};
int evict(SET s){
int index_to_evict=0;
int min_time=s.blocks[0].time_use;
for(int i=1;i<4;i++){
if(min_time>s.blocks[i].time_use){
    index_to_evict=i;
    min_time=s.blocks[i].time_use;
}
}
return index_to_evict;
}
memresp MDR;
class miss_status_holding_register{
    public:
    queue<cpureq> MSHRq;
    //this should do the memreq?
    void add_req(cpureq CPUreq){
        MSHRq.push(CPUreq);
        memreq MEMreq;
        MEMreq.addr=CPUreq.addr;
        MEMreq.t=read;
        MDR=mem.service(MEMreq);
    }
    cpureq serviced_req(){
        cpureq request_being_serviced=MSHRq.front();
        MSHRq.pop();
        return request_being_serviced;
    }
}MSHR;
class WRITE_BUFFER{
    public:
    queue<pair<block,int>> b;
    void add_writeback(pair<block,int> req){
        b.push(req);
        memreq MEMreq;
        MEMreq.addr=req.second;
        MEMreq.t=write;
        MEMreq.data=req.first;
        mem.service(MEMreq);
        b.pop();
    }
}write_buffer;
class CACHE{
    public:
    miss_status_holding_register MSHR;
    int way=4;
    SET cache[64];
    
    cpuresp service(cpureq CPUreq){
        auto currentTimePoint=chrono::system_clock::now();
        auto currentTime = std::chrono::time_point_cast<chrono::seconds>(currentTimePoint);
        auto seconds = currentTime.time_since_epoch().count();
        SET s=cache[CPUreq.index()];
        cpuresp CPUresp;
        CPUresp.addr=CPUreq.addr;
        for(int i=0;i<way;i++){
            info_block b=s.blocks[i];
            if(b.tag==CPUreq.tag() && (b.s==V||b.s==M)){
                if(CPUreq.t==read){
                    globalOutputStream<<CPUreq.addr<<" "<<"read"<<" "<<"Hit"<<endl;
                    CPUresp.data=b.blck.block_elements[CPUreq.offset()];
                    b.time_use=seconds;//for LRU replacement
                    return CPUresp;
                }
                if(CPUreq.t==write){
                    globalOutputStream<<CPUreq.addr<<" "<<"write"<<" "<<"Hit"<<endl;
                    b.s=M;
                    b.blck.block_elements[CPUreq.offset()]=CPUreq.data;
                    b.time_use=seconds;//for LRU replacement
                    cache[CPUreq.index()].blocks[i]=b;
                    return CPUresp;
                }
            }
            if(b.tag==CPUreq.tag() && b.s==MP){
                //pass
            }
        }
        if(CPUreq.t==read){
            globalOutputStream<<"### "<<CPUreq.addr<<" "<<"read"<<" "<<"Miss ###"<<endl;
        }
        if(CPUreq.t==write){
            globalOutputStream<<"### "<<CPUreq.addr<<" "<<"write"<<" "<<"Miss ###"<<endl;
        }   

        int eviction_index=evict(s);
        if(s.blocks[eviction_index].s==M){
            globalOutputStream<<"for req "<<CPUreq.addr<<" "<<"block_evicted addr: "<<(s.blocks[eviction_index].tag<<6+CPUreq.index())<<endl;
            /*
            add the evicted block to writeback buffer
            addr=s.block[eviction_index]<<6(tag left shifted)+index
            */
            write_buffer.add_writeback(make_pair(s.blocks[eviction_index].blck,s.blocks[eviction_index].tag<<6+CPUreq.index()));
        }
        s.blocks[eviction_index].s=MP;
        s.blocks[eviction_index].tag=CPUreq.tag();
        //add the block and the address to write buffer at eviction
        
    
        //send the memory request here-->to miss status holding register
        MSHR.add_req(CPUreq);
        //when MDR gets filled fill the cache and also give CPUresp
        cpureq cpu_request_being_services=MSHR.serviced_req();
        if(cpu_request_being_services.t==read){
            s.blocks[eviction_index].s=V;
            s.blocks[eviction_index].blck=MDR.data;
            s.blocks[eviction_index].time_use=seconds;//for LRU replacement
            cache[cpu_request_being_services.index()]=s;
            CPUresp.t=cpu_request_being_services.t;
            CPUreq.data=MDR.data.block_elements[cpu_request_being_services.offset()];
            return CPUresp;
        }
        if(cpu_request_being_services.t==write){
            s.blocks[eviction_index].s=M;
            s.blocks[eviction_index].blck=MDR.data;
            s.blocks[eviction_index].time_use=seconds;//for LRU replacement
            s.blocks[eviction_index].blck.block_elements[CPUreq.offset()]=CPUreq.data;
            cache[cpu_request_being_services.index()]=s;
        }
        
    }
}cache;
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
class Forwarder{
public:
//no state
//get the data dependency in decode stage and check for the required values in MEM and Execute stage
bool cant_resolve(int rs);
void resolve();
}forwarder;

map<int,string> IM;
//IM
int GPR[32];
int ins[32];
//GPR
int Prev_val_of_ins_rdl;

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
    bool stall;
}PC;
struct IFID_register_set{
    public:
    string IR;
    int DPC;
    bool valid;
    bool stall;
}ifid;

typedef struct func_bits{
    string opcode;
    string func3;
    string func7;  

}func;

struct  IDEX_register_set{
    int DPC;
    string instruction;
    int immf;
    controller cw;
    int rs1;
    int rs2;
    func func_b;
    int rdl;
    bool valid;
    bool stall;
    int rsl1;
    int rsl2;
}idex;


struct EXMO_register_set{
    int DPC;
    controller cw;
    string instruction;
    int ALU_out;
    int rs2;
    int rdl;
    bool valid;
    bool stall;
    bool is_branch_taken;
}exmo;

struct MOWB_register_set{
    int DPC;
    string instruction;
    controller cw;
    int ld_out;
    int alu_out;
    int rdl;
    bool valid;
    bool stall;
}mowb;
// global NPC
int NPC;
int TPC;
string s(31,'0');
bool Forwarder::cant_resolve(int rs){
//see the dependency in decode stage(the same detection system may work)  
//can't resolve when the value is from a load instruction in execute stage
if(rs==idex.rdl && idex.cw.cw_map["regwrite"] && idex.cw.cw_map["memread"]){
    return true;
}
return false;
//whatever is in the execute stage if it is load and it writes into 
//that register then the forwarder can't resolve
}
void Forwarder::resolve(){
    //detect and resolve any dependency
    //forwarding from exmo register set
    
    bool rs1_forwarded_by_exmo=false;
    bool rs2_forwarded_by_exmo=false;
    if(exmo.valid && idex.valid){
    if(exmo.rdl==idex.rsl1 && exmo.rdl!=0&& exmo.cw.cw_map["regwrite"] && !exmo.cw.cw_map["memread"]){
    //if the registers match and the source wants to write and the output is available right now
            idex.rs1=exmo.ALU_out;
            rs1_forwarded_by_exmo=true;
        }
    if(exmo.rdl==idex.rsl2 && exmo.rdl!=0&& exmo.cw.cw_map["regwrite"] &&!exmo.cw.cw_map["memread"] && (!idex.cw.cw_map["ALUsrc"]||idex.cw.cw_map["memwrite"])) {
    //rest same forward just for branch and R-R operation
        idex.rs2=exmo.ALU_out;
        rs2_forwarded_by_exmo=true;
    }
    }
    if(mowb.valid && idex.valid){
    if(mowb.rdl==idex.rsl1 && mowb.rdl!=0 && mowb.cw.cw_map["regwrite"] && !rs1_forwarded_by_exmo){
        if(mowb.cw.cw_map["memreg"]){
            idex.rs1=mowb.ld_out;
        }
        else{
            idex.rs1=mowb.alu_out;
        }
    }
    if(mowb.rdl==idex.rsl2 && mowb.rdl!=0 && mowb.cw.cw_map["regwrite"] && (!idex.cw.cw_map["ALUsrc"]||idex.cw.cw_map["memwrite"]) && !rs2_forwarded_by_exmo){
        if(mowb.cw.cw_map["memreg"]){
            idex.rs2=mowb.ld_out;
        }
        else{
            idex.rs2=mowb.alu_out;
        }
    }
    }
}


void fetch(){
if (!PC.valid || ifid.stall){return;}
    string instruction=IM[PC.pc];

    if(instruction==s){PC.valid=false;}
    ifid.IR=instruction;
    ifid.DPC=PC.pc;
    
    ifid.valid=true;
    return;
}
void add_bubble_in_execute(){
    //for bubble:
    string nop="00000000000000000000000000010011";
    
    string opcode=nop.substr(25,7);
    string func3=nop.substr(17,3);
    string func7=nop.substr(0,7);
    controller cw(opcode);
    idex.cw=cw;
    idex.func_b.func3=func3;
    idex.func_b.func7=func7;
    idex.func_b.opcode=opcode;
    idex.instruction=nop;
    idex.rdl=0;
    idex.DPC=-1;
    idex.valid=true;
}
void add_bubble_in_decode(){
    //for bubble:
    string nop="00000000000000000000000000010011";
    ifid.DPC=-1;
    ifid.stall=false;
    ifid.IR=nop;
    ifid.valid=true;
}
void decode(){
    if(!ifid.valid ||idex.stall){return;}
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
    if(cw.cw_map["regread"] ){
        if(ins[rsl1]==-1){
            rs1=GPR[rsl1];    
        }
        else if(forwarder.cant_resolve(rsl1)){
            ifid.stall=true;
            add_bubble_in_execute();
            return;
        }
        //if forwarder can resolve move ahead
    }
      
    if(cw.cw_map["regread"] &&  (opcode=="0110011"|| opcode=="0100011"||opcode=="1100011")){//checks is R type or store or branch
        if(ins[rsl2]==-1){
            rs2=GPR[rsl2];  
        }
    else if(forwarder.cant_resolve(rsl2)){
            ifid.stall=true;
            add_bubble_in_execute();
            return;
        }
    }

    if(cw.cw_map["regwrite"] && rd!=0){
        //but if the next instruction is a brach this may get flushed --> restore to previous value in that case
        Prev_val_of_ins_rdl=ins[rd];
        //if writes into the rd register 
        ins[rd]=PC_decode;//then lock the register
    }
    
    if(cw.cw_map["memwrite"]){
        idex.immf=stii(imms);
    }
    else if(cw.cw_map["branch"]){
        idex.immf=stii(immb);
    }
    else{
        idex.immf=stii(immil);
    }

    string func3=instruction.substr(17,3);
    string func7=instruction.substr(0,7);

    idex.instruction=instruction;
    idex.DPC=PC_decode;
    idex.cw=cw;
    idex.rs1=rs1;
    idex.rs2=rs2;
    idex.func_b.func3=func3;
    idex.func_b.func7=func7;
    idex.func_b.opcode=opcode;
    idex.rdl=rd;
    idex.rsl1=rsl1;
    idex.rsl2=rsl2;
    idex.valid=true;
    ifid.stall=false;
    return;
}
void execute(){
    if(!idex.valid||exmo.stall){return;}
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
            if(zero_flag){TPC=BPC;
            exmo.is_branch_taken=true;
            }
            else{TPC=NPC;
            exmo.is_branch_taken=false;
            }
            break;
            case 5:
            if(zero_flag||gt_flag){TPC=BPC;
            exmo.is_branch_taken=true;
            }
            else{TPC=NPC;
            exmo.is_branch_taken=false;
            }
            break;
            case 7:
            if(lt_flag){TPC=BPC;
            exmo.is_branch_taken=true;
            }
            else{TPC=NPC;
            exmo.is_branch_taken=false;
            }
            break;
        }
    }else{
        TPC=NPC;
        exmo.is_branch_taken=false;
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
    if(!exmo.valid||mowb.stall){return;}
    string instruction=exmo.instruction;
    if(instruction==s){exmo.valid=false;}
    controller cw=exmo.cw;
    int ALU_result=exmo.ALU_out;
    int rd=exmo.rdl;
    int rs2=exmo.rs2;
    int ldresult;
    if(instruction!=s){
    if(cw.cw_map["memread"]){
        auto start = chrono::high_resolution_clock::now();   
        cpureq reqLoad;
        reqLoad.addr=ALU_result;
        reqLoad.t=read;
        cpuresp respload=cache.service(reqLoad);
        auto stop = chrono::high_resolution_clock::now();
        ldresult=respload.data;
        auto duration = chrono::duration_cast<chrono::microseconds>(stop - start);
        globalOutputStream<< duration.count() << endl;
    }
    if(cw.cw_map["memwrite"]){
       auto start = chrono::high_resolution_clock::now();   
        cpureq reqstore;
        reqstore.data=rs2;
        reqstore.addr=ALU_result;
        reqstore.t=write;
        cpuresp respstrore=cache.service(reqstore);
        auto stop = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(stop - start);
        globalOutputStream<< duration.count() << endl;
    }
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
    int PC_writeback=mowb.DPC;
    int ALU_result=mowb.alu_out;
    int ldresult=mowb.ld_out;
    controller cw=mowb.cw;
    int rd=mowb.rdl;
    int write_res;
      if(cw.cw_map["regwrite"]){
        if(cw.cw_map["memreg"]){
            write_res=ldresult;
        }else{
             write_res=ALU_result;
        }
        if(ins[rd]==PC_writeback && rd!=0){
            GPR[rd]=write_res;
            ins[rd]=-1;//unlock
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
globalOutputStream.open("cache_output.txt");
s=s+"1";
make_IM_GPR_INS();
//load the pipeline
PC.valid=true;
PC.pc=0;
int CC=0;
while(PC.valid||ifid.valid||idex.valid||exmo.valid||mowb.valid){
    
    /*
    code to print timing diagram at the beginning of every cycle
    */
    int PC_val[5]={0};
    if(PC.valid){PC_val[0]=PC.pc;}else{PC_val[0]=-2;}
    if(ifid.valid){PC_val[1]=ifid.DPC;}else{PC_val[1]=-2;}
    if(idex.valid){PC_val[2]=idex.DPC;}else{PC_val[2]=-2;}
    if(exmo.valid){PC_val[3]=exmo.DPC;}else{PC_val[3]=-2;}
    if(mowb.valid){PC_val[4]=mowb.DPC;}else{PC_val[4]=-2;}
    CC+=1;
    if (CC == 1) {
    cout << left << setw(6) << "cycle"<<setw(8) << "fetch"<<setw(8) << "decode"<<setw(8) << "execute"<<setw(8) << "memory"<< "writeback" << endl;
    }
    cout<<left<<setw(8)<<CC<<setw(8)<<PC_val[0]<<setw(8)<<PC_val[1]<<setw(8)<<PC_val[2]<<setw(8)<<PC_val[3]<<PC_val[4]<<endl;
    /*
    END
    */
    NPC=PC.pc+4;
    forwarder.resolve();
    wb();
    memory();
    execute();
    decode();
    fetch();
    if(!ifid.stall){//if ifid stalls dont change the pc
    if(CC<3){
            PC.pc=NPC;
        }
        else{
            PC.pc=TPC;
            /*
            if a branch is setting this and the branch is taken you should fetch the next instruction 
            because it is going to be some valid instruction else make the PC.valid remain false if it is
            the terminating set of instructions
            */
           if(exmo.cw.cw_map["branch"] && exmo.is_branch_taken){
            PC.valid=true;
           }
            
        }
    }
    //if branch taken --> flush 
    if(exmo.cw.cw_map["branch"] && exmo.is_branch_taken){
        //ifid.flush and idex.flush
        if(idex.rdl!=0){
            ins[idex.rdl]=Prev_val_of_ins_rdl;
        }
        
        add_bubble_in_decode();
        add_bubble_in_execute();
        //PC.valid=true;
    }
}
for(int i=0;i<=31;i++){
    cout<<"GPR["<<i<<"]: "<<GPR[i]<<endl;
}
}

