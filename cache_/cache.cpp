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
/*
address-->int so 32 bits 
#sets=64-->index bits=6
block size=64-->offset=6
tag bits=32-6-6//mujhe kya maine to int hi le liya

offset=addr&0x3F
index=addr>>6&0x3F
*/

int main(){
    globalOutputStream.open("cache_output.txt");
    for(int i=0;i<16*4;i++){//populate firsT 5 sets by read request
        auto start = chrono::high_resolution_clock::now();   
        cpureq req1;
        req1.data=i;
        req1.addr=i*4;
        req1.t=write;
        cpuresp resp1=cache.service(req1);
        auto stop = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(stop - start);
        globalOutputStream<< duration.count() << endl;
    }
    for(int i=1;i<5;i++){
        auto start = chrono::high_resolution_clock::now();   
        cpureq req1;
        req1.data=i*64*64;
        req1.addr=i*64*64;//issue 4 write request to index 1
        req1.t=write;
        cpuresp resp1=cache.service(req1);
        auto stop = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(stop - start);
        globalOutputStream<< duration.count() << endl;
    }
block block1=mem.mm[0];
for(auto i:block1.block_elements){
    cout<<block1.block_elements[i]<<endl;
}
}