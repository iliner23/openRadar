#ifndef OPENCTREE_H
#define OPENCTREE_H

#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <string>
#include <iostream>
#include <algorithm>
#include <filesystem>

class openCtree
{
private:
   struct index_header{
        int32_t offset_of_file;//0x0
        int32_t offset_written;//0x8
        int32_t resource;//0xC
        int32_t active_entries;//0x18
        int32_t bTree_root;//0x1C
        uint16_t node_size;//0x2A
        uint8_t index_type;//0x39 fixed = 0, leaded = 4, padded = 8, combinated = 12
        bool dublicate;//0x3A
        char padding;//0x3C
        uint8_t ptrSize;//0x3F
        int16_t key_length;//0x42
        int16_t members_count;//0x44
        int16_t member_number;//0x46
        const char * altSeq = nullptr;
        ~index_header();
    };

    mutable std::ifstream _dat;
    mutable std::ifstream _idx;
    std::vector<index_header> _header;
    int16_t _index = 0;
    uint16_t _reclen = 0;
    mutable std::string _lastKey;

    struct{
        uint32_t _nextHope = 0;
        uint16_t _byteSize = 0;
        uint64_t _leafPtr = 0;
        uint64_t _basePtr = 0;
    } mutable _navigate;

    std::string gtData() const;
public:
    openCtree(const std::string&);
    openCtree() = default;
    void open(const std::string&);
    void close() noexcept;
    bool is_open() const noexcept;
    bool is_dublicate() const;
    bool is_alternate() const;
    int16_t membersCount() const;
    void setMember(const int16_t);
    int32_t dataEntries() const;
    uint16_t getReclen() const;
    std::string getData(const uint32_t) const;
    std::string getData(std::string) const;
    std::string next() const;
    std::string getLastKey() const;
    std::string convertString(const std::string&) const;
    char getPaddingChar() const;
};



#endif // OPENCTREE_H
