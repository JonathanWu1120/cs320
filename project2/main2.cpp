#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#define ERR_320_ARGS 1
#define ERR_320_FILE 2

class abs_cache {
    protected:
        std::vector<unsigned> _cache;
        unsigned num_hits, num_acc;
    public:
        abs_cache(unsigned sz) : _cache(sz, -1), num_hits(0), num_acc(0) { }
        virtual void process(bool, unsigned addr) {
            auto& c_addr = _cache[addr % _cache.size()];
            if(c_addr == addr) ++num_hits;
            else c_addr = addr;
            ++num_acc;
        }
        virtual std::string results() const {
            return std::to_string(num_hits) + "," + std::to_string(num_acc) + ";";
        }
};

template <bool NoWrMiss = false, bool FetchMiss = false>
class set_cache : public abs_cache {
    protected:
        std::vector<unsigned> _acc_times;
        unsigned num_slots; bool last_result;
        bool find_slot(bool wr, bool, unsigned addr, unsigned set) {
            unsigned evictee = set*num_slots;
            for(auto i = evictee; i < (set+1)*num_slots; ++i) {
                if(_cache[i] == addr) {
                    _acc_times[i] = num_acc+1;
                    return true;
                }
                if(_acc_times[i] < _acc_times[evictee]) evictee = i;
            }
            if(!NoWrMiss || !wr) {
                _acc_times[evictee] = num_acc+1;
                _cache[evictee] = addr;
            }
            return false;
        }
    public:
        set_cache(unsigned sz, unsigned num_slots) : abs_cache(sz),
            _acc_times(sz, 0), num_slots(num_slots), last_result(false) { }
        virtual void process(bool wr, unsigned addr) override {
            unsigned set = addr % (_cache.size() / num_slots);
            if((last_result = find_slot(wr, true, addr, set))) ++num_hits;
            ++num_acc;
        }
        void prefetch(bool wr, unsigned addr) {
            if(!FetchMiss || !last_result) { // num_acc manip. for time correctness
                --num_acc; unsigned set = (++addr) % (_cache.size() / num_slots);
                find_slot(wr, false, addr, set); ++num_acc;
            }
        }
};

class hc_cache : public abs_cache { // fully associative
    protected:
        std::vector<unsigned> _hc_bits;
    public:
        hc_cache() : abs_cache(512), _hc_bits(512,0) { }
        void process(bool, unsigned addr) override {
            int index = -1;
            for(unsigned i = 0; i < _cache.size(); ++i)
                if(_cache[i] == addr ) { index = i; break; }
            if(index == -1) {
                index = 0;
                for(int shamt = 8; shamt > -1; --shamt)
                     if((_hc_bits[index] & (1 << shamt)) > 0) index += 1 << shamt;
                _cache[index] = addr;
            }
            else ++num_hits;
            for(int shamt = 0, div = 2; shamt < 9; div *= 2, ++shamt) {
                int other_index = (index / div) * div;
                if(index == other_index) other_index += div / 2;
                if((_hc_bits[index] & (1 << shamt)) == 0) {
                    _hc_bits[index] ^= 1 << shamt;
                    if((_hc_bits[other_index] & (1 << shamt)) > 0)
                        _hc_bits[other_index] ^= 1 << shamt;
                }
                if(index % div != 0) index = other_index;
            }
            ++num_acc;
        }
};

template <typename CacheType>
void put_results(std::ofstream& ofs, CacheType sets[4]) {
    for(int i = 0; i < 3; ++i) ofs << sets[i].results() << " ";
    ofs << sets[3].results() << std::endl;
}

int main(int argc, char** argv) {
    if(argc != 3) {
        std::cerr << "ERROR: Need only input and output filenames" << std::endl;
        exit(ERR_320_ARGS);
    }
    std::ifstream ifs; std::ofstream ofs;
    ifs.open(argv[1], std::ios::in); ofs.open(argv[2], std::ios::out);
    if(!ifs.is_open() || !ofs.is_open()) {
        std::cerr << "ERROR: Cannot open file(s)" << std::endl;
        exit(ERR_320_FILE);
    }
    abs_cache dmaps[4] = {{32}, {128}, {512}, {1024}};
    set_cache<> sets[4] = {{512,2}, {512,4}, {512,8}, {512,16}};
    set_cache<true> wmsets[4] = {{512,2}, {512,4}, {512,8}, {512,16}};
    set_cache<false, false> nfsets[4] = {{512,2}, {512,4}, {512,8}, {512,16}};
    set_cache<false, true> mfsets[4] = {{512,2}, {512,4}, {512,8}, {512,16}};
    set_cache<> full{512,512};
    hc_cache hc;
    char type; std::string addr;
    while(ifs >> type >> addr) {
        unsigned ll_addr = strtoll(addr.c_str(), nullptr, 16) >> 5; // Get rid of offset bits
        bool wr = type == 'S';
        for(int i = 0; i < 4; ++i) {
            dmaps[i].process(wr, ll_addr); sets[i].process(wr, ll_addr);
            wmsets[i].process(wr, ll_addr);
            nfsets[i].process(wr, ll_addr); mfsets[i].process(wr, ll_addr);
            nfsets[i].prefetch(wr, ll_addr); mfsets[i].prefetch(wr, ll_addr);
        }
        full.process(wr, ll_addr);
        hc.process(wr, ll_addr);
    }
    put_results(ofs, dmaps); put_results(ofs, sets);
    ofs << full.results() << std::endl; ofs << hc.results() << std::endl;
    put_results(ofs, wmsets); put_results(ofs, nfsets); put_results(ofs, mfsets);
    ifs.close(); ofs.close();
} 
