#include<iostream>
#include<vector>
#include<cmath>
#include<sstream>
#include<fstream>
using namespace std;

int nik;

int numberSets;
int BlocksperSet;
int WordsperBlock;
int BytesperWord;
string allocate_status;
string write_scheme;
string evict_method;

int num_idx_bits;
int num_tag_bits;
int num_block_offset;
int num_byte_offset;
string index_str;
string tag_str;
string block_offset_str;
string byte_offset_str;

int total_loads = 0;
int total_stores = 0;
int load_misses = 0;
int store_misses = 0;
int load_hits = 0;
int store_hits = 0;
int total_cycles = 0;


void rev_string(string &s){
    string temp = s;
    s = "";
    for(int i=0; i<int(temp.size()); i++){
        s = temp[i] + s;
    }
}

string hexadecimal_to_binary(string hex){
    string binary = "";
    for (char c : hex) {
        switch (c) {
            case '0': binary += "0000"; break;
            case '1': binary += "0001"; break;
            case '2': binary += "0010"; break;
            case '3': binary += "0011"; break;
            case '4': binary += "0100"; break;
            case '5': binary += "0101"; break;
            case '6': binary += "0110"; break;
            case '7': binary += "0111"; break;
            case '8': binary += "1000"; break;
            case '9': binary += "1001"; break;
            case 'A': binary += "1010"; break;
            case 'B': binary += "1011"; break;
            case 'C': binary += "1100"; break;
            case 'D': binary += "1101"; break;
            case 'E': binary += "1110"; break;
            case 'F': binary += "1111"; break;
            case 'a': binary += "1010"; break;
            case 'b': binary += "1011"; break;
            case 'c': binary += "1100"; break;
            case 'd': binary += "1101"; break;
            case 'e': binary += "1110"; break;
            case 'f': binary += "1111"; break;
            default: std::cerr << "Invalid hex digit '" << c << "'"; return "";
        }
    }
    return binary;
}

int binary_to_decimal(string &binary){
    int decimal = 0;
    int power = 0;
    for (int i = binary.length() - 1; i >= 0; --i) {
        if (binary[i] == '1') {
            decimal += std::pow(2, power);
        }
        power++;
    }
    return decimal;
}

class Byte{
    public:
    string data;
    string byte_address;

    Byte(){
        byte_address = "Dummy address";
        data = "Dummy Data stored";
    }

    ~Byte(){
        data = "";
        byte_address = "";
    }
};

class Word{
    public:
    string address; //from this address till the next four addresses
    vector<Byte*> word;

    Word(){
        address = "";
    }

    ~Word(){
        for(int i=0; i<BytesperWord; i++){
            delete word[i];
        } //check if this desturction is correct, from the vector directly
    }
};

class Block{
    public:
    string tag_value;
    bool valid_bit;
    bool dirty_bit;
    // vector<Word*> block;
    string words;

    Block(){
        tag_value = "";
        valid_bit = false;
        dirty_bit = false;
        words = "No data stored in blocks for now";
    }

};

class Set{
    public:
    int set_size;
    int max_set_size;
    vector<Block*> set;

    Set(){
        // cout<<"entered set constructor"<<endl;
        set_size = 0;
        max_set_size = BlocksperSet;
        // cout<<"exited set constructor"<<endl;
    };

    ~Set(){
        for(int i=0; i<set_size; i++){
            delete set[i];
        }
    };

    bool check_hit(int &idx){
        for(int i=0; i<set_size; i++){
            if(set[i]->valid_bit == true && set[i]->tag_value == tag_str){
                idx = i;
                return true;
            }
        }
        idx = -1;
        return false;
    };

    int get_lru(){
        if(set_size == 0){
            return 0;
        }
        return set_size - 1;
    }

    void read_for_lru(bool hit, int block_idx){
        // cout<<"Entered read_for_lru function  -->  ";
        if(hit == true){
            // cout<<" Read Hit  -- and block index : "<<block_idx<<endl;
            total_cycles++; 
            load_hits++;
            // -------------1---------------------this change did not change the result
            // the block_idx cannot be -1, since it is a hit
            Block* recent_block = set[block_idx];
            Block* newblock = new Block();
            newblock->tag_value = recent_block->tag_value;
            newblock->valid_bit = recent_block->valid_bit;
            newblock->dirty_bit = recent_block->dirty_bit;
            newblock->words = newblock->words;
            set.erase(set.begin() + block_idx);
            set.insert(set.begin(), newblock);
            // cout<<"Block placed in the most recent place "<<endl;
            delete recent_block;
        }else{
            // in case of miss
            // cout<<" Read miss  -- and block index : "<<block_idx<<endl;
            total_cycles = total_cycles + 1 + (100*WordsperBlock); 
            load_misses++;
            // cout<<" The lru_index is : "<<lru_index<<"  -->  ";
            if(set_size == max_set_size){
                // cout<<"set full, need new space, so eviction required"<<endl;
                // there is no space
                //remove least recently used block, which is the last block
                // if write-back then write the data if dirty bit is on
                if(write_scheme == "write-back" and allocate_status == "write-allocate"){
                    if(set[set_size - 1]->dirty_bit == true){
                        // cout<<"the scheme is write-back and write-allocate and the dirty bit is on"<<endl;
                        total_cycles = total_cycles + (100*WordsperBlock);
                    }
                }
                Block* newblock = new Block();
                newblock->tag_value = tag_str;
                newblock->valid_bit = true;
                newblock->dirty_bit = false;
                set.insert(set.begin(), newblock);
                // cout<<"new block inserted at the start of the set"<<endl;
                Block* last_block = set.back();
                set.pop_back();
                delete last_block;
                // cout<<"This case crossed"<<endl;
            }else{
                // cout<<"set has space : set index : "<<binary_to_decimal(index_str)<<endl;
                set_size++;
                // load from memory at the start of the vector
                Block* newblock = new Block();
                newblock->tag_value = tag_str;
                newblock->valid_bit = true;
                newblock->dirty_bit = false;
                set.insert(set.begin(), newblock);
            }
        } 
        // cout<<" Reading Completed using lru"<<endl;
    }

    void read_for_fifo(bool hit, int block_idx){
        block_idx++;
        block_idx--;
        // cout<<"Entered read_for_fifo function  -->  ";
        if(hit == true){
            // cout<<" Read Hit  -->  ";
            total_cycles++; 
            load_hits++;
        }else{
            // load from memory
            // cout<<" Read miss  -->  ";
            total_cycles = total_cycles + 1 + (100*WordsperBlock); 
            load_misses++;
            if(set_size == max_set_size){
                // set is full
                // now remove the last element and add in the first
                if(write_scheme == "write-back" and allocate_status == "write-allocate"){
                    if(set[set_size - 1]->dirty_bit == true){
                        total_cycles = total_cycles + (100*WordsperBlock);
                    }
                }else{
                    // write-through scheme is followed
                }
                Block* newblock = new Block();
                newblock->tag_value = tag_str;
                newblock->valid_bit = true;
                newblock->dirty_bit = false;
                set.insert(set.begin(), newblock);
                Block* last_block = set.back();
                set.pop_back();
                delete last_block;
            }else{
                // load from memory in the empty space
                set_size++;
                Block* newblock = new Block();
                newblock->tag_value = tag_str;
                newblock->valid_bit = true;
                newblock->dirty_bit = false;
                set.insert(set.begin(), newblock);
            }
        }
        // cout<<" Reading Completed using fifo"<<endl;
    }

    void write_for_lru(bool hit, int block_idx){
        // cout<<"Entered read_for_write function  -->  ";
        if(hit == true){
            store_hits++;
            if(write_scheme == "write-through"){
                // cout<<"Write-Hit with the config = write-through with any allocate"<<endl;
                // write in memory
                total_cycles = total_cycles + 1 + 100;
                // check if here one more cycle has to be added
            }else{
                // cout<<"Write-Hit with the config = write-back with any allocate"<<endl;
                // if already the dirty bit is on of off does not matter, update in cache only
                // if dirty bit is off then switch it on
                set[block_idx]->dirty_bit = true;
                total_cycles++;
            }
            Block* recent_block = set[block_idx];
            Block* newblock = new Block();
            newblock->tag_value = recent_block->tag_value;
            newblock->valid_bit = recent_block->valid_bit;
            newblock->dirty_bit = recent_block->dirty_bit;
            newblock->words = newblock->words;
            set.erase(set.begin() + block_idx);
            set.insert(set.begin(), newblock);
            delete recent_block;
        }else{
            store_misses++;
            if(write_scheme == "write-through"){
                if(allocate_status == "write-allocate"){
                    // cout<<"Write-Miss with the config = write-through with write-allocate"<<endl;
                    // bring the data to cache
                    // write in both cache and memory
                    total_cycles = total_cycles + 1 + (100*WordsperBlock) + (100);
                    Block* newblock = new Block();
                    newblock->tag_value = tag_str;
                    newblock->valid_bit = true;
                    newblock->dirty_bit = false;
                    if(set_size == max_set_size){
                        // set is full, remove the least recently used block
                        Block* oldblock = set.back();
                        set.pop_back();
                        set_size--;
                        delete oldblock;
                    }
                    set_size++;
                    set.insert(set.begin(), newblock);
                    // check if there should be 100 more cycles added in this case
                }else{
                    // cout<<"Write-Miss with the config = write-through with no-write-allocate"<<endl;
                    // allocate-status == "no-write-allocate"
                    total_cycles = total_cycles + 100;
                }
            }else{
                // write-scheme == "write-back"
                if(allocate_status == "write-allocate"){
                    // cout<<"Write-Miss with the config = write-back with write-allocate"<<endl;
                    // write back with write allocate, data not present in the cache
                    // so we bring the data from memory to cache and update only in cache and set its dirty bit to 1
                    total_cycles = total_cycles + 1 + (100*WordsperBlock);
                    if(set_size == max_set_size){
                        // no space
                        // delete the block at the end, and write its data if the dirty bit is on
                        Block* end_block = set.back();
                        if(end_block->dirty_bit == true){
                            total_cycles = total_cycles + (100*WordsperBlock);
                        }
                        set_size--;
                        set.pop_back();
                        // delete end_block;
                    }else{
                        // space is there 
                    }
                    Block* newblock = new Block();
                    newblock->tag_value = tag_str;
                    newblock->valid_bit = true;
                    newblock->dirty_bit = true;
                    set.insert(set.begin(), newblock);
                    set_size++;
                    // check here also if 1 more cycle has to added for the writing in the cache
                }else{
                    // cout<<"Write-Miss with the config = write-back with no-write-allocate"<<endl;
                    total_cycles = total_cycles + 1 + (100*WordsperBlock);
                }
            }
        }
    }

    void write_for_fifo(bool hit, int block_idx){
        if(hit == true){
            store_hits++;
            if(write_scheme == "write-through"){
                // write in memory
                total_cycles = total_cycles + 101;
                // check if here one more cycle has to be added
            }else{
                // if already the dirty bit is on of off does not matter, update in cache only
                // if dirty bit is off then switch it on
                set[block_idx]->dirty_bit = true;
                total_cycles++;
            }
        }else{
            store_misses++;
            if(write_scheme == "write-through"){
                if(allocate_status == "write-allocate"){
                    // bring the data to cache
                    // write in both cache and memory
                    if(set_size == max_set_size){
                        Block* old_block = set.back();
                        set.pop_back();
                        set_size--;
                        delete old_block;
                    }
                    Block* newblock = new Block();
                    newblock->tag_value = tag_str;
                    newblock->valid_bit = true;
                    newblock->dirty_bit = false;
                    set.insert(set.begin(), newblock);
                    set_size++;
                    total_cycles = total_cycles + 1 + (100*WordsperBlock) + (100);
                    // check if there should be 100 more cycles added in this case
                }else{
                    // allocate-status == "no-write-allocate"
                    total_cycles = total_cycles + 100;
                }
            }else{
                // write-scheme == "write-back"
                if(allocate_status == "write-allocate"){
                    // write back with write allocate, data not present in the cache
                    // so we bring the data from memory to cache and update only in cache and set its dirty bit to 1
                    if(set_size == max_set_size){
                        // no space, we have to remove a element which is at th eend
                        Block* old_block = set.back();
                        if(old_block->dirty_bit == true){
                            total_cycles = total_cycles + (100*WordsperBlock);
                        }
                        set.pop_back();
                        set_size--;
                        delete old_block;
                    }
                    Block* newblock = new Block();
                    total_cycles = total_cycles + 1 + (100*WordsperBlock);
                    newblock->tag_value = tag_str;
                    newblock->valid_bit = true;
                    newblock->dirty_bit = true;
                    set_size++;
                    set.insert(set.begin(), newblock);
                    // check here also if 1 more cycle has to added for the writing in the cache
                }else{
                    total_cycles = total_cycles + 1 + (100*WordsperBlock);
                }
            }
        }
    }

    void print_out(string operation, bool hit){
        if(hit == true){
            if (operation == "read"){
                cout<<"load hit";
            }else{
                cout<<"store hit";
            }
        }else{
            if(operation == "read"){
                cout<<"load miss";
            }else{
                cout<<"store miss";
            }
        }
    }


    void SetExecute(string operation){
        // cout<<"Entered SetExecute function  ->  ";
        int block_idx = 0;
        bool hit = check_hit(block_idx);
        // nik++;
        // cout<<nik<<") ";
        // print_out(operation, hit);
        // cout<<endl;
        // cout<<"The hit status is : "<<hit<<" and the value of block_idx is : "<<block_idx<<"  -->  ";
        if(operation == "read"){
            total_loads++;
            // cout<<"Performing the operation : read  -->  ";
            if(evict_method == "lru"){
                // cout<<"Evict method : lru  -->  "<<endl;
                read_for_lru(hit, block_idx);
            }
            else{
                // cout<<"Evict method : fifo  -->  "<<endl;
                //  here the evict method is fifo
                read_for_fifo(hit, block_idx);
            }
        }else{
            total_stores++;
            // cout<<"Performing the operation : write  -->  ";
            if(evict_method == "lru"){
                // cout<<"Evict method : lru  -->  "<<endl;
                write_for_lru(hit, block_idx);
            }else{
                // cout<<"Evict method : fifo  -->  "<<endl;
                write_for_fifo(hit, block_idx);
            }
        }
    }
};

class Cache{
    public:
    vector<Set*> cache;

    Cache(){
        // cout<<"entered cache constructor and num sets : "<<numberSets<<endl;
        for(int i=0; i<numberSets; i++){
            // cout<<i+1;
            Set* set = new Set();
            cache.push_back(set);
        }
        // cout<<"exited cache constructor"<<endl;
    }

    ~Cache(){
        for(int i=0; i<numberSets; i++){
            delete this->cache[i];
        }
    }

    void parse_address(string address){
        rev_string(address);
        byte_offset_str = address.substr(0,num_byte_offset);
        block_offset_str = address.substr(num_byte_offset, num_block_offset);
        index_str = address.substr(num_byte_offset + num_block_offset, num_idx_bits);
        tag_str = address.substr(num_byte_offset + num_block_offset + num_idx_bits, num_tag_bits);
        rev_string(byte_offset_str);
        rev_string(block_offset_str);
        rev_string(index_str);
        rev_string(tag_str);
        // cout<<"Parsed address : "<<tag_str<<"  "<<index_str<<"  "<<block_offset_str<<"  "<<byte_offset_str<<endl;
    }

    void execute(string address, string operation){
        // cout<<"Parse address into fields  ->  "<<endl;
        parse_address(address);
        int idx;
        if ( index_str == ""){
            idx = 0;
        }
        else{
            idx = binary_to_decimal(index_str);
        }
        this->cache[idx]->SetExecute(operation);
    }
};

void print_stats(){
    // cout<<"--------------------------------------------------------"<<endl;
    cout<<"Total loads: "<<total_loads<<endl;
    cout<<"Total stores: "<<total_stores<<endl;
    cout<<"Load hits: "<<load_hits<<endl;
    cout<<"Load misses: "<<load_misses<<endl;
    cout<<"Store hits: "<<store_hits<<endl;
    cout<<"Store misses: "<<store_misses<<endl;
    cout<<"Total cycles: "<<total_cycles<<endl;
    // cout<<"--------------------------------------------------------"<<endl;
}

void print_all_info(Cache *c){
    cout<<"----------------------------------------------------------------"<<endl;
    cout<<"The number of sets : "<<numberSets<<endl;
    cout<<"The number of blocks in one set : "<<BlocksperSet<<endl;
    cout<<"The number of words in one block : "<<WordsperBlock<<endl;
    cout<<"The write-scheme is : "<<write_scheme<<endl;
    cout<<"The write allocate status is : "<<allocate_status<<endl;
    cout<<"The type of eviction method is : "<<evict_method<<endl;
    cout<<"----------------------------------------------------------------"<<endl;
    cout<<"The size of the cache vector(of sets) is : "<<c->cache.size()<<endl;
    cout<<"The size of the 1 set vector(of blocks) is : "<<c->cache[0]->set.size()<<endl;
    // cout<<"The size of the 1 block vector(of words) is : "<<c->cache[0]->max_set_size<<endl;
    cout<<"The number of index bits is : "<<num_idx_bits<<endl;
    cout<<"The number of tag bits is : "<<num_tag_bits<<endl;
    cout<<"The number of bits for blocks offset is : "<<num_block_offset<<endl;
    // cout<<"The data stored in the first byte is : "<<c->cache[0]->set[0]->block[0]->word[0]->data<<endl;
    cout<<"----------------------------------------------------------------"<<endl;
}

/*
vector<vector<string>> read_file(string filename){
    vector<vector<string>> test_case;
    ifstream trace_file(filename);
    if (!trace_file.is_open() ){
        std::cerr<<"Error while opening the trace file"<<endl;
    }else{
    string line;
    int i = 0;
    while(getline(trace_file, line)){
        istringstream ss(line);
        string instr_type, adrs, num;
        if(ss >> instr_type >> adrs >>  num){
            // cout<<"instruction type: "<<instr_type<<"   address: "<<adrs<<"   number: "<<num<<endl;
            vector<string> temp {instr_type, adrs, num};
            test_case.push_back(temp);
        }
        i++;
    }
    trace_file.close();
    return test_case;
    }
}

vector<vector<string>> read_file(){
    string a, b, c;
    while(cin >> a >> b >> c){
        vector<string> line_input;
        line_input.push_back(a);
        line_input.push_back(b);
        line_input.push_back(c);
    }
}

void code_runner(Cache *c){
    vector<vector<string>> test_case;
    test_case = read_file();
    for(int i=0; i<test_case.size(); i++){
        string type = test_case[i][0];
        string address = test_case[i][1];
        address = address.substr(2, address.size() - 2);
        while(address.size() < 8){
            address = '0' + address;
        }
        address = hexadecimal_to_binary(address);
        string num = test_case[i][2];
        if(type == "l"){
            //load instruction
            // cout<<num<<")  read from address : "<<address<<" of length : "<<address.size()<<endl;
            c->execute(address, "read");
        }else{
            //store instruction
            // cout<<num<<")  write to address : "<<address<<" of length : "<<address.size()<<endl;
            c->execute(address, "write");
        }
    }
}
*/

int main(int argc, char* argv[]){
    // cout<<"eneter"<<endl;
    if( argc != 7){
        cout<<"number of inputs is wrong"<<endl;
    }
    // cout<<"starting"<<endl;
    numberSets = stoi(argv[1]);
    BlocksperSet = stoi(argv[2]);
    WordsperBlock = stoi(argv[3])/4;
    BytesperWord = 4;
    allocate_status = argv[4];
    write_scheme = argv[5];
    evict_method = argv[6];
    // cout<<"variables made"<<endl;
    num_byte_offset = 2;
    num_block_offset = log2(WordsperBlock);
    num_idx_bits = log2(numberSets);
    num_tag_bits = 32 - (num_block_offset + num_byte_offset + num_idx_bits);
    // cout<<"number of bits decided"<<endl;
    Cache *c = new Cache();
    // cout<<"cache initialized"<<endl;
    // print_all_info(c);
    string address;
    string type;
    string num;
    nik = 0;
    while(cin >> type >> address >> num){
        nik++;
        // cout<<"input taken : "<<type<<"  "<<address<<"  "<<num<<endl;
        int n = address.size() - 2;
        address = address.substr(2, n);
        while(address.size() < 8){
            address = "0" + address;
        }
        address = hexadecimal_to_binary(address);
        if(type == "l"){
            //load instruction
            // cout<<nik<<")  read from address : "<<address<<" of length : "<<address.size()<<endl;
            c->execute(address, "read");
        }else{
            //store instruction
            // cout<<nik<<")  write to address : "<<address<<" of length : "<<address.size()<<endl;
            c->execute(address, "write");
        }
    }

    // code_runner(c);
    print_stats();

    delete c;
    return 0;
}