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
bool find(std::vector<std::vector<std::vector<unsigned>>> &set_ass,std::vector<int> &sets,bool lmiss,std::tuple<std::string,std::string> a,int i,bool prefetch){
	unsigned addr = strtoll(std::get<1>(a).c_str(), nullptr,16) >> 5;
	if(prefetch) ++addr;
	std::string ls = std::get<0>(a);
	unsigned set2 = addr%set_ass[i].size();
	bool in = false; 
	auto it = set_ass[i][set2].begin();
	for(unsigned j = 0; j < set_ass[i][set2].size(); j++){
		if(set_ass[i][set2][j] == addr){
			in = true;	
			if(!prefetch){
				sets[i]++;
			}
			set_ass[i][set2].erase(it);
			set_ass[i][set2].push_back(addr);
			return true;
		}
		it++;
	}
	if(!in){
		if(lmiss && ls == "L"){
			set_ass[i][set2].erase(set_ass[i][set2].begin());
			set_ass[i][set2].emplace_back(addr);
		}else if(!lmiss){
			set_ass[i][set2].erase(set_ass[i][set2].begin());
			set_ass[i][set2].emplace_back(addr);
		}
	}
	return false;
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
	//direct
	std::vector<std::vector<unsigned>> direct;
	direct.push_back(std::vector<unsigned>(32,-1));
	direct.push_back(std::vector<unsigned>(128,-1));
	direct.push_back(std::vector<unsigned>(512,-1));
	direct.push_back(std::vector<unsigned>(1024,-1));
	std::vector<int> direct_hit(4,0);
	std::vector<int> sets1(4,0);
	//full associative
	std::vector<unsigned> full_ass(1,-1);
	std::vector<unsigned> full_ass2(1,-1);
	std::vector<unsigned> full_count(512,0);
	int full1 = 0;
	int full2 = 0;
	//set associative
	std::vector<std::vector<std::vector<unsigned>>> set_ass;
	std::vector<std::vector<std::vector<unsigned>>> set_ass2;
	std::vector<std::vector<std::vector<unsigned>>> set_ass3;
	std::vector<std::vector<std::vector<unsigned>>> set_ass4;
	for(int i = 2; i < 17; i*=2){
	set_ass.push_back(std::vector<std::vector<unsigned>>(512/i,std::vector<unsigned>(i,-1)));
	set_ass2.push_back(std::vector<std::vector<unsigned>>(512/i,std::vector<unsigned>(i,-1)));
	set_ass3.push_back(std::vector<std::vector<unsigned>>(512/i,std::vector<unsigned>(i,-1)));
	set_ass4.push_back(std::vector<std::vector<unsigned>>(512/i,std::vector<unsigned>(i,-1)));
	}
	std::vector<int> sets2(4,0);
	std::vector<int> sets3(4,0);
	std::vector<int> sets4(4,0);
	int sz = input.size();
	for(int x = 0; x < sz;x++){
		if(x == sz/4){
			std::cout << "25% " << std::endl;
		}else if(x == sz/2){
			std::cout << "50% " << std::endl;
		}else if(x == sz/4 * 3){
			std::cout << "75% " << std::endl;
		}
		auto a = input[x];
		unsigned addr = strtoll(std::get<1>(a).c_str(), nullptr,16) >> 5;
		std::string ls = std::get<0>(a);
		for(int i = 0; i < 4; i ++){
			//direct
			if(direct[i][addr%direct[i].size()] == addr){
				direct_hit[i]++;
			}else{
				direct[i][addr%direct[i].size()] = addr;
			}
			//set associative
			find(set_ass,sets1,false,a,i,false);
			//Write miss set associative
			find(set_ass2,sets2,true,a,i,false);
			//prefetch1
			find(set_ass3,sets3,false,a,i,false);
			find(set_ass3,sets3,false,a,i,true);
			//prefetch2;
			if(!find(set_ass4,sets4,false,a,i,false)){
				find(set_ass4,sets4,false,a,i,true);
			}
		}
		//fully associative lru
		bool added = false;
		auto it = full_ass.begin();
		for(int i = 0; i < full_ass.size();i++){
			if(full_ass[i] == addr){
				full1++;
				added = true;
				full_ass.erase(it);
				full_ass.push_back(addr);
				break;
			}
			it++;
		}
		if(!added){
			if(full_ass.size() < 512){
				full_ass.push_back(addr);
			}else{
				full_ass.erase(full_ass.begin());
				full_ass.push_back(addr);	
			}
		}
		//full associative hot cold
		added = false;
		for(int i = 0; i < full_ass2.size();i++){
			if(full_ass2[i] == addr){
				full2++;
				added = true;
				full_count[i]++;
				break;
			}
		}
		if(!added){
			int min = 1000000;
			int index = 0;
			for(int i = 0; i < full_ass2.size();i++){
				if(full_ass2[i] < min){
					index = i;
					min = full_ass2[i];
				}
			}
			full_ass2[index] = addr;
			full_count[index] = 0;
		}
	}
	out.open(argv[2]);
	out << direct_hit[0] << "," << sz << "; ";
	out << direct_hit[1] << "," << sz << "; ";
	out << direct_hit[2] << "," << sz << "; ";
	out << direct_hit[3] << "," << sz << ";";
	out << '\n'; 
	out << sets1[0] << "," << sz << "; ";
	out << sets1[1] << "," << sz << "; ";
	out << sets1[2] << "," << sz << "; ";
	out << sets1[3] << "," << sz << ";";
	out << '\n'; 
	out << full1 << "," << sz << ";" << std::endl;
	out << full2 << "," << sz << ";" << std::endl;
	out << sets2[0] << "," << sz << "; ";
	out << sets2[1] << "," << sz << "; ";
	out << sets2[2] << "," << sz << "; ";
	out << sets2[3] << "," << sz << ";";
	out << '\n';
	out << sets3[0] << "," << sz << "; ";
	out << sets3[1] << "," << sz << "; ";
	out << sets3[2] << "," << sz << "; ";
	out << sets3[3] << "," << sz << ";";
	out << '\n';
	out << sets4[0] << "," << sz << "; ";
	out << sets4[1] << "," << sz << "; ";
	out << sets4[2] << "," << sz << "; ";
	out << sets4[3] << "," << sz << ";";
	out << '\n';
	out.close();
	std::cout << "Done!" <<std::endl;
	return 0;
}	
