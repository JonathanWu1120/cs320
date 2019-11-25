#include <iostream>
#include <math.h>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>
#include <fstream>

std::ofstream out;

unsigned get_addr(std::string s){
	unsigned x;
	std::stringstream ss;
	std::string addr = s.substr(2);
	ss << addr;
	ss >> std::hex >> x;
	return x;
}
int main(int argc, char** argv){
	if(argc!= 3){
		std::cerr << "Please format as ./cache-sim input.txt output.txt";
		return 0;
	}
	std::string line;
	std::ifstream infile(argv[1]);
	std::vector<std::tuple<std::string,std::string>> input;
	while(std::getline(infile,line)){
		std::istringstream iss(line);
		std::string a, b;
		if(!(iss >> a >> b)){
			break;
		}
		auto x = std::make_tuple(a,b);
		input.push_back(x);
	}
	out.open(argv[2]);
	std::vector<std::vector<unsigned>> direct;
	direct.push_back(std::vector<unsigned>(32,-1));
	direct.push_back(std::vector<unsigned>(128,-1));
	direct.push_back(std::vector<unsigned>(512,-1));
	direct.push_back(std::vector<unsigned>(1024,-1));
	std::vector<int> direct_hit(4,0);

	std::vector<std::vector<std::vector<unsigned>>> set_ass;
	set_ass.push_back(std::vector<std::vector<unsigned>>(256,std::vector<unsigned>(2,-1)));
	set_ass.push_back(std::vector<std::vector<unsigned>>(128,std::vector<unsigned>(4,-1)));
	set_ass.push_back(std::vector<std::vector<unsigned>>(64,std::vector<unsigned>(8,-1)));
	set_ass.push_back(std::vector<std::vector<unsigned>>(32,std::vector<unsigned>(16,-1)));

	std::vector<int> sets1(4,0);
	for(auto a : input){
		unsigned addr = strtoll(std::get<1>(a).c_str(), nullptr,16) >> 5;
		//direct
		for(int i = 0; i < 4; i++){
			if(direct[i][addr%direct[i].size()] == addr){
				direct_hit[i]++;
			}else{
				direct[i][addr%direct[i].size()] = addr;
			}
		}
		//set associative
		for(int i = 0; i < 4; i ++){
			unsigned set2 = addr%set_ass[i].size();
			bool in = false; 
			auto it = set_ass[i][set2].begin();
			for(unsigned j = 0; j < set_ass[i][set2].size(); j++){
				if(set_ass[i][set2][j] == addr){
					in = true;	
					sets1[i]++;
					set_ass[i][set2].erase(it);
					set_ass[i][set2].push_back(addr);
					break;
				}
				it++;
			}
			if(!in){
				set_ass[i][set2].erase(set_ass[i][set2].begin());
				set_ass[i][set2].emplace_back(addr);
			}
		}

		
	}
	for(int i : sets1){
	std::cout << i << std::endl;
	}
/*
	for(int i : direct_hit){
		std::cout << i << std::endl;
	}
	*/
	return 0;
}	
