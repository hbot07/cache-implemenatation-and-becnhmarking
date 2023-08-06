#include <stdio.h>
#include "stdc++.hpp"
#include <cmath>
#include <bitset>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip> // for std::hex conversion

using namespace std;

class item {                      //item represents one block of data.It is represented by its Tag. Usage is used to keep track of 
public:                              // the last time this block was used. This is used to enforce the eviction policy.
    int state;                        // state is used to check if the block is dirty
    uint64_t Tag;
    int usage;
};

class L2cache {
public:
    // Data  Members
    uint64_t sets;
    uint64_t ways;
    uint64_t blocksize;
    uint64_t totalsize;
    std::vector<std::vector<item>> data;
    uint64_t reads, read_misses, writes, write_misses, writebacks, tag_bits, set_bits, offset_bits;
    uint64_t tag_mask, set_mask;             //used to get set bits and tag bits from the location

    // Member Functions()
    L2cache(uint64_t w, uint64_t b, uint64_t t) {
        //initializes L2 Cache
        offset_bits = log2(b);
        reads = 0;
        read_misses = 0;
        writes = 0;
        write_misses = 0;
        writebacks = 0;

        std::vector<item> temp;
        item temp1;
        sets = (t / w) / b;
        ways = w;
        blocksize = b;
        totalsize = t;
        set_bits = log2(sets);
        tag_bits = (64 - set_bits) - offset_bits;
        set_mask = (sets - 1) << offset_bits;
        tag_mask = 0xffffffffffffffff - (set_mask + blocksize - 1);
        for (uint64_t i = 0; i < sets; i++) {
            for (uint64_t j = 0; j < ways; j++) {
                temp1.state = -1;
                temp1.Tag = 0;
                temp1.usage = -1;
                temp.push_back(temp1);
            }
            data.push_back(temp);

        }

    }

    void update_usage(uint64_t set_id, uint64_t way) {
        for (uint64_t i = 0; i < ways; i++) {
            if (i ==
                way) { continue; }                                  // This will be called after each read or write access.
            if (data[set_id][i].usage ==
                -1) { continue; }               // It will set the usage of the newly accessed block to 0 and increase the usage of other blocks by one(within the same set)
            data[set_id][i].usage += 1;

        }
    }

    uint64_t get_LU(uint64_t set_id) {
        int maximum = -1;
        int max_way = -1;
        for (uint64_t i = 0; i <
                             ways; i++) {                         // Takes a particular set number and finds the least recently used block in that set.

            if (data[set_id][i].usage > maximum) {
                maximum = data[set_id][i].usage;
                max_way = i;
            }
        }
        return max_way;
    }


    void process_read(uint64_t location) {

        uint64_t set_id = (location & set_mask) >> offset_bits;

        uint64_t tag_id = (location & tag_mask) >> (offset_bits + set_bits);

        // int way = -1;
        reads += 1;
        int flag = 0;
        uint64_t target_way;
        for (uint64_t j = 0; j < ways; j++) {
            if ((data[set_id][j].Tag == tag_id) &&
                ((data[set_id][j].state != -1))) {          // Case 1: cache hit. update the usage and thats it
                data[set_id][j].usage = 0;
                update_usage(set_id, j);
                flag = 1;

                break;
            }

        }


        if (!flag) {

            read_misses += 1;
            for (uint64_t j = 0; j < ways; j++) {
                if (data[set_id][j].state == -1) {
                    data[set_id][j].Tag = tag_id;                                   // Case 2: Cache miss but there is empty space in the set.
                    data[set_id][j].usage = 0;
                    data[set_id][j].state = 0;
                    update_usage(set_id, j);
                    flag = 1;

                    break;
                }

            }

        }


        if (!flag) {

            target_way = get_LU(set_id);

            if (data[set_id][target_way].state == 1) {
                writebacks += 1;
            }
            data[set_id][target_way].Tag = tag_id;
            data[set_id][target_way].usage = 0;                            // Case 3 : Cache miss and there is no more space in the set. Get the least recently used block 
            data[set_id][target_way].state = 0;                           // and writeback if required 
            update_usage(set_id, target_way);                             // replace it with the new block
            flag = 1;
        }
    }

    void process_write(uint64_t location) {

        uint64_t set_id = (location & set_mask) >> offset_bits;

        uint64_t tag_id = (location & tag_mask) >> (offset_bits + set_bits);

        //int way = -1;
        writes += 1;
        int flag = 0;
        uint64_t target_way;
        for (int j = 0; j < ways; j++) {
            if ((data[set_id][j].Tag == tag_id) && ((data[set_id][j].state != -1))) {
                data[set_id][j].usage = 0;                                                // Note that the state is set to 1. reads dont change the state but writes can.
                data[set_id][j].state = 1;
                update_usage(set_id, j);
                flag = 1;

                break;
            }

        }


        if (!flag) {

            write_misses += 1;
            for (uint64_t j = 0; j < ways; j++) {
                if (data[set_id][j].state == -1) {
                    data[set_id][j].Tag = tag_id;
                    data[set_id][j].usage = 0;
                    data[set_id][j].state = 1;
                    update_usage(set_id, j);
                    flag = 1;

                    break;
                }

            }

        }


        if (!flag) {

            target_way = get_LU(set_id);

            if (data[set_id][target_way].state == 1) {
                writebacks += 1;
            }
            data[set_id][target_way].Tag = tag_id;
            data[set_id][target_way].usage = 0;
            data[set_id][target_way].state = 1;
            update_usage(set_id, target_way);
            flag = 1;
        }
    }
};

class L1Cache {
    // Access specifier
public:
    // Data  Members
    uint64_t sets;
    uint64_t ways;
    uint64_t blocksize;
    uint64_t totalsize;
    L2cache *L2;
    std::vector<std::vector<item>> data;
    uint64_t reads, read_misses, writes, write_misses, writebacks, tag_bits, set_bits, offset_bits;
    uint64_t tag_mask, set_mask;

    // Member Functions()
    L1Cache(uint64_t w, uint64_t b, uint64_t t, L2cache *l) {
        offset_bits = log2(b);
        L2 = l;
        reads = 0;
        read_misses = 0;
        writes = 0;
        write_misses = 0;
        writebacks = 0;

        std::vector<item> temp;
        item temp1;
        sets = (t / w) / b;
        ways = w;
        blocksize = b;
        totalsize = t;
        set_bits = log2(sets);
        tag_bits = (64 - set_bits) - offset_bits;
        set_mask = (sets - 1) << offset_bits;
        tag_mask = 0xffffffffffffffff - (set_mask + blocksize - 1);
        for (uint64_t i = 0; i < sets; i++) {
            for (uint64_t j = 0; j < ways; j++) {
                temp1.state = -1;
                temp1.Tag = 0;
                temp1.usage = -1;
                temp.push_back(temp1);
            }
            data.push_back(temp);

        }

    }

    void update_usage(uint64_t set_id, uint64_t way) {
        for (uint64_t i = 0; i < ways; i++) {
            if (i == way) { continue; }
            if (data[set_id][i].usage == -1) { continue; }
            data[set_id][i].usage += 1;

        }
    }

    uint64_t get_LU(uint64_t set_id) {
        int maximum = -1;
        int max_way = -1;
        for (uint64_t i = 0; i < ways; i++) {

            if (data[set_id][i].usage > maximum) {
                maximum = data[set_id][i].usage;
                max_way = i;
            }
        }
        return max_way;
    }


    void process_read(uint64_t location) {

        uint64_t set_id = (location & set_mask) >> offset_bits;

        uint64_t tag_id = (location & tag_mask) >> (offset_bits + set_bits);

        // int way = -1;
        reads += 1;
        int flag = 0;
        uint64_t target_way;
        for (uint64_t j = 0; j < ways; j++) {
            if ((data[set_id][j].Tag == tag_id) && ((data[set_id][j].state != -1))) {
                data[set_id][j].usage = 0;
                update_usage(set_id, j);
                flag = 1;

                break;
            }

        }


        if (!flag) {

            read_misses += 1;
            for (uint64_t j = 0; j < ways; j++) {
                if (data[set_id][j].state == -1) {
                    L2->process_read(
                            location);                              // Simulates a read miss by calling the L2 cache for that data
                    data[set_id][j].Tag = tag_id;
                    data[set_id][j].usage = 0;
                    data[set_id][j].state = 0;
                    update_usage(set_id, j);
                    flag = 1;
                    break;
                }

            }

        }


        if (!flag) {

            target_way = get_LU(set_id);

            if (data[set_id][target_way].state == 1) {
                writebacks += 1;
                L2->process_write(                                          // simulates a writeback by writing to L2
                        (data[set_id][target_way].Tag << (set_bits + offset_bits)) + (set_id << offset_bits) + 1);
            }
            L2->process_read(location);
            data[set_id][target_way].Tag = tag_id;
            data[set_id][target_way].usage = 0;
            data[set_id][target_way].state = 0;
            update_usage(set_id, target_way);
            flag = 1;
        }
    }

    void process_write(uint64_t location) {               //Similar to above cases

        uint64_t set_id = (location & set_mask) >> offset_bits;

        uint64_t tag_id = (location & tag_mask) >> (offset_bits + set_bits);

        //int way = -1;
        writes += 1;
        int flag = 0;
        uint64_t target_way;
        for (uint64_t j = 0; j < ways; j++) {
            if ((data[set_id][j].Tag == tag_id) && ((data[set_id][j].state != -1))) {
                data[set_id][j].usage = 0;
                data[set_id][j].state = 1;
                update_usage(set_id, j);
                flag = 1;

                break;
            }

        }


        if (!flag) {
            write_misses += 1;
            for (uint64_t j = 0; j < ways; j++) {
                if (data[set_id][j].state == -1) {
                    L2->process_read(location);
                    data[set_id][j].Tag = tag_id;
                    data[set_id][j].usage = 0;
                    data[set_id][j].state = 1;
                    update_usage(set_id, j);
                    flag = 1;
                    break;
                }

            }

        }


        if (!flag) {
            target_way = get_LU(set_id);
            if (data[set_id][target_way].state == 1) {
                writebacks += 1;
                L2->process_write(
                        (data[set_id][target_way].Tag << (set_bits + offset_bits)) + (set_id << offset_bits) + 1);
            }
            L2->process_read(location);
            data[set_id][target_way].Tag = tag_id;
            data[set_id][target_way].usage = 0;
            data[set_id][target_way].state = 1;
            update_usage(set_id, target_way);
            flag = 1;

        }
    }

};


int main(int argc, char *argv[]) {
    if (argc != 7) {
        std::cerr << "Usage: ./cache_simulate <BLOCKSIZE> <L1_SIZE> <L1_ASSOC> <L2_SIZE> <L2_ASSOC> <trace_file>"
                  << std::endl;
        return 1;
    }

    uint64_t blockSize = std::stoi(argv[1]);
    uint64_t L1Size = std::stoi(argv[2]);                     //taking command line inputs
    uint64_t L1Assoc = std::stoi(argv[3]);
    uint64_t L2Size = std::stoi(argv[4]);
    uint64_t L2Assoc = std::stoi(argv[5]);
    std::string traceFile = argv[6];

//    if (!isPowerOfTwo(blockSize) || !isPowerOfTwo(L1Size) || !isPowerOfTwo(L1Assoc) ||
//        !isPowerOfTwo(L2Size) || !isPowerOfTwo(L2Assoc)) {
//        std::cerr << "All parameters should be powers of 2." << std::endl;
//        return 1;
//    }

    L2cache L2(L2Assoc, blockSize, L2Size);        //initialize cache objects
    L1Cache L1(L1Assoc, blockSize, L1Size,
               &L2);     // Enter the modifiable parameters of the cache. L1(ways, blocksize, totalsize, pointer to the L2cache object)

    std::ifstream file(traceFile);

    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            char operation;
            uint64_t address;                                               //take input from file
            if (iss >> operation >> std::hex >> address) {
                if (operation == 'r') {                                 //send requests to L1.
                    L1.process_read(address);
                } else if (operation == 'w') {
                    L1.process_write(address);
                }

            }
        }
        file.close();
    } else {
        std::cerr << "Failed to open the file :(." << std::endl;
        return 1;
    }
    std::cout << "===== Simulation Results =====" << std::endl;
    std::cout << "i. number of L1 reads: " << L1.reads << "\n";
    std::cout << "ii. number of L1 read misses: " << L1.read_misses << "\n";
    std::cout << "iii. number of L1 writes: " << L1.writes << "\n";
    std::cout << "iv. number of L1 write misses: " << L1.write_misses << "\n";
    std::cout << "v. L1 miss rate: " << (float) (L1.read_misses + L1.write_misses) / (L1.reads + L1.writes) << "\n";
    std::cout << "vi. number of writebacks from L1 memory: " << L1.writebacks << "\n";
    std::cout << "vii. number of L2 reads: " << L2.reads << "\n";
    std::cout << "viii. number of L2 read misses: " << L2.read_misses << "\n";
    std::cout << "ix. number of L2 writes: " << L2.writes << "\n";
    std::cout << "x. number of L2 write misses: " << L2.write_misses << "\n";
    std::cout << "xi. L2 miss rate: " << (float) (L2.read_misses + L2.write_misses) / (L2.reads + L2.writes) << "\n";
    std::cout << "xii. number of writebacks from L2 memory: " << L2.writebacks << "\n";
    int time = 1 * (L1.reads + L1.writes) + 20 * (L2.reads + L2.writes) + 200 * (L2.read_misses + L2.writebacks);
    std::cout << "Time: " << time << "\n";

    return 0;

}
