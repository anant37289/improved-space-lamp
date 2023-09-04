
#include<map>
#include<vector>
#include<string>
using namespace std;
#ifndef INSTRUCTIONMAP_H // include guard
#define INSTRUCTIONMAP_H

namespace N{
    class instr_map{
    map<string,vector<string>> instr;
 public:
   instr_map();
   vector<string> get_info(string operation);
};
}

#endif 
