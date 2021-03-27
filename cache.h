#ifndef CACHE_H
#define CACHE_H
#include <unordered_map>

struct cache{
    std::unordered_map<uint16_t, std::string> _cacheRemed;
    std::unordered_map<uint16_t, std::string> _cacheAuthor;
    uint16_t _lenRem, _lenAuthor;
};
#endif // CACHE_H
