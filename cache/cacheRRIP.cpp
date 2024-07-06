#include <bits/stdc++.h>
using namespace std;
int read_miss=0;
int read_hit=0;
int write_miss=0;
int write_hit=0;
class block{
public:
int data[4];
};
class MM{
public:
block arr[1<<26];
block readService(int address){
    return arr[address>>6];
};
void writeService(int address,block b){
    arr[address>>6]=b;
};
}mem;
enum state{I,V,MP,M};
class cache_{
public:
/*
4-way set associative cache 16 KB
*/
block data_arr[64][4];
int tag_arr[64][4];
state state_arr[64][4];
int RRPV[64][4];
cache_(){
    for(int i=0;i<64;i++){
        for(int j=0;j<4;j++){
            RRPV[i][j]=INT_MAX;
            state_arr[i][j]=I;
        }

    }
}
int readService(int address){
  
    int index=address>>6 & 0x3F;
    int tag=address>>12;
    int offset=address & 0x3F;
    //for this set
    for(int i=0;i<4;i++){
        if(tag_arr[index][i]==tag && (state_arr[index][i]==V||state_arr[index][i]==M)){
            read_hit+=1;
            RRPV[index][i]=0;
            return data_arr[index][i].data[offset>>4];//return the integer indexed values
        }
        if(tag_arr[index][i]==tag && state_arr[index][i]==MP){
            //pass
        }
    }
        //if no tag match then eviction
        read_miss+=1;
        int max_rrpv=RRPV[index][0];
        int evict=0;
        for(int i=1;i<4;i++){
            if(RRPV[index][i]>max_rrpv){
                evict=i;
                max_rrpv=RRPV[index][i];
            }
        }
        if(state_arr[index][evict]==M){
            mem.writeService(((tag_arr[index][evict] & 0xFFFFF)<<12)+(index<<6),data_arr[index][evict]);
        }
        state_arr[index][evict]=MP;
        tag_arr[index][evict]=tag;
        data_arr[index][evict]=mem.readService(address);
        state_arr[index][evict]=V;
        RRPV[index][evict]=INT_MAX-1;
        return data_arr[index][evict].data[offset>>4];
    
}
void writeService(int address,int data){

    int index=address>>6 & 0x3F;
    int tag=address>>12;
    int offset=address & 0x3F;
    //for this set
    for(int i=0;i<4;i++){
        if(tag_arr[index][i]==tag && (state_arr[index][i]==V||state_arr[index][i]==M)){
            write_hit+=1;
            RRPV[index][i]=0;
            state_arr[index][i]=M;
            data_arr[index][i].data[offset>>4]=data;//return the integer indexed values
            return;
        }
        if(tag_arr[index][i]==tag && state_arr[index][i]==MP){
            //pass
        }
        }
        //if no tag match then eviction
        write_miss+=1;
        int max_rrpv=RRPV[index][0];
        int evict=0;
        for(int i=1;i<4;i++){
            if(RRPV[index][i]>max_rrpv){
                evict=i;
                max_rrpv=RRPV[index][i];
            }
        }
        if(state_arr[index][evict]==M){
            mem.writeService(((tag_arr[index][evict])<<12)+(index<<6),data_arr[index][evict]);
        }

        state_arr[index][evict]=MP;
        tag_arr[index][evict]=tag;
        data_arr[index][evict]=mem.readService(address);
        data_arr[index][evict].data[offset>>4]=data;
        state_arr[index][evict]=M;
        RRPV[index][evict]=INT_MAX-1;
}
}cache;
int main(){
    freopen("trace.txt","r",stdin);
    string request;
   while(getline(cin,request)){
    int address=stoul(request.substr(4,4+8),0,16);

    if(stoi(request.substr(2,3))==0){
        cache.readService(address);
    }
    else{
        cache.writeService(address,-1);
    }
   }
   cout<<"read_miss: "<<read_miss<<endl;
   cout<<"read_hit: "<<read_hit<<endl;
   cout<<"write_miss: "<<write_miss<<endl;
   cout<<"write_hit: "<<write_hit<<endl;
   cout<<"read_hit_rate: "<<(float)read_hit/(read_hit+read_miss)<<endl;
   cout<<"write_hit_rate: "<<(float)write_hit/(write_hit+write_miss)<<endl;
}