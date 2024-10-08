#ifndef OPENCTREE_H
#define OPENCTREE_H

#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <stdexcept>
#include <string>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <variant>
#include <array>
#include <map>

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
        std::shared_ptr<char[]> altSeq;
    };

    mutable std::ifstream _dat;
    std::ifstream _idx;
    std::vector<index_header> _header;
    std::shared_ptr<
        std::map<uint64_t, uint64_t, std::greater<>>> _leaf;//first - index pos, second - basePtr

    uint16_t _reclen = 0;

    std::string _fileName;

    struct{
        uint16_t index = 0;
        std::string lastKey;
        uint64_t lastValuePos = 0;
        uint64_t lastPosition = std::numeric_limits<uint64_t>::max();
        uint32_t nextHope = 0;
        uint16_t byteSize = 0;
        uint64_t leafPtr = 0;
        uint64_t basePtr = 0;
    } _navigate;

    inline std::string gtData(const uint64_t) const;
    inline std::string readOrNot(const bool, const uint64_t);
    inline std::string uncompressString(const std::string &);
    inline std::variant<bool, std::string> commonAtKey(std::string, const bool, const bool, const bool);
    inline void writeLeafs();
public:
    openCtree(const std::string & filename) { open(filename); }
    openCtree() = default;
    openCtree(const openCtree &);
    openCtree(openCtree &&) = default;

    openCtree & operator=(const openCtree &);
    openCtree & operator=(openCtree &&) = default;

    void open(const std::string&);
    void close() noexcept;
    bool isOpen() const noexcept;

    bool isIteratorSet() const noexcept;
    bool isDublicateKey() const;
    bool isAlternateSequence() const;

    uint16_t indexCount() const;
    void setIndex(const uint16_t);
    uint16_t index() const;

    int32_t size() const;

    uint16_t serviceDataLenght() const;

    std::string at(const uint32_t, const bool readDbText = true);
    std::string at(std::string, const bool readDbText = true);
    std::string firstFind(std::string, const bool readDbText = true);

    std::string next(const bool readDbText = true);
    std::string front(const bool readDbText = true);
    std::string back(const bool readDbText = true);

    std::string key() const;
    std::string encodeKey(const std::string&) const;
    std::string decodeKey(const std::string&) const;

    std::string currentValue() const;

    bool haveKey(std::string);
    uint16_t keyLenght() const;
    uint8_t pointerSize() const;

    uint64_t currentPosition();

    char paddingChar() const;
};
#endif // OPENCTREE_H
