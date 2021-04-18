#include "openctree.h"

char openCtree::paddingChar() const{
    if(!isOpen())
        throw std::logic_error("Database isn't open");

    return _header[_index].padding;
}
std::string openCtree::key() const{
    if(!isOpen())
        throw std::logic_error("Database isn't open");

    if(_lastKey.empty())
        throw std::logic_error("Function getData or next haven't been call");

    return _lastKey;
}
bool openCtree::isAlternateSequence() const{
    if(!isOpen())
        throw std::logic_error("Database isn't open");

    if(_header[_index].altSeq == nullptr)
        return false;

    return true;
}
std::string openCtree::convertKey(const std::string & key) const{
    if(!isAlternateSequence())
        throw std::logic_error("Member hasn't an alternate sequence");

    auto str = key;
    const auto ptr = _header[_index].altSeq;

    for(auto & it : str){
        it = ptr[(uint8_t)it];
    }

    return str;
}
openCtree::index_header::~index_header(){
    if(altSeq != nullptr)
        delete [] altSeq;
}
std::string openCtree::gtData(){
    uint8_t sig[2];
    _dat.read((char *) &sig, 2);

    constexpr const uint8_t sg[] = {250, 250};
    if(memcmp(sig, sg, 2) != 0)
        throw std::runtime_error("Incorrect signature");

    uint32_t dtSize;
    _dat.seekg((uint64_t) _dat.tellg() + 4);
    _dat.read((char *) &dtSize, sizeof(dtSize));
    std::string rawData(dtSize, '\0');
    _dat.read(rawData.data(), dtSize);

    if(!_dat.good())
        throw std::runtime_error("Database was destroyed");

    return rawData;
}
bool openCtree::isDublicateKey() const{
    if(!isOpen())
        throw std::logic_error("Database isn't open");

    return _header[_index].dublicate;
}
openCtree::openCtree(const std::string & filename){
    open(filename);
}

void openCtree::open(const std::string & filename){
    if(isOpen())
        close();

    _idx.open(filename + ".idx", std::ios::in | std::ios::binary);
    _dat.open(filename + ".dat", std::ios::in | std::ios::binary);
    _idx.sync_with_stdio(false);
    _dat.sync_with_stdio(false);

    if(!_idx.is_open() || !_dat.is_open())
        throw std::runtime_error("Database isn't open");

    int16_t members = std::numeric_limits<int16_t>::max();

    for(int16_t i = 0; i != members; i++){
        index_header ih;
        uint64_t ptr = _idx.tellg();

        _idx.read((char *) &ih.offset_of_file, sizeof(ih.offset_of_file));
        _idx.seekg(ptr + 0x8);
        _idx.read((char *) &ih.offset_written, sizeof(ih.offset_written));
        _idx.seekg(ptr + 0x18);
        _idx.read((char *) &ih.active_entries, sizeof(ih.active_entries));
        _idx.seekg(ptr + 0x1c);
        _idx.read((char *) &ih.bTree_root, sizeof(ih.bTree_root));
        _idx.seekg(ptr + 0x2a);
        _idx.read((char *) &ih.node_size, sizeof(ih.node_size));
        _idx.seekg(ptr + 0x39);
        _idx.read((char *) &ih.index_type, sizeof(ih.index_type));
        _idx.seekg(ptr + 0x3a);
        _idx.read((char *) &ih.dublicate, sizeof(ih.dublicate));
        _idx.seekg(ptr + 0x3c);
        _idx.read((char *) &ih.padding, sizeof(ih.padding));
        _idx.seekg(ptr + 0x3f);
        _idx.read((char *) &ih.ptrSize, sizeof(ih.ptrSize));
        _idx.seekg(ptr + 0x42);
        _idx.read((char *) &ih.key_length, sizeof(ih.key_length));
        _idx.seekg(ptr + 0x44);
        _idx.read((char *) &ih.members_count, sizeof(ih.members_count));
        _idx.seekg(ptr + 0x46);
        _idx.read((char *) &ih.member_number, sizeof(ih.member_number));

        _idx.seekg(ptr + 0x80);

        if(i == 0)
            members = ih.members_count + 1;

        if(ih.ptrSize != 4)
            throw std::logic_error("Programm doesn't support databases with pointer size more than 4 bytes");

        if(!_idx.good())
            throw std::runtime_error("Database was destroyed");

        _header.push_back(ih);
    }

    uint64_t ptrS;
    _idx.seekg(0xC);
    _idx.read((char *) &ptrS, sizeof(ptrS));
    const auto baseS = ptrS;

    if(ptrS != 0)
    {
        for(auto i = 0; i != 2; ++i){
            _idx.seekg(ptrS);
            char magic[2];//{'\xFE', '\xFE'};
            _idx.read((char *) magic, 2);

            if(memcmp(magic, "\xFE\xFE", 2) != 0)
                throw std::runtime_error("Error magic sequence");

            if(i == 0){
                _idx.seekg(ptrS + 0xE);
                _idx.read((char *) &ptrS, _header[0].ptrSize);

                if(ptrS == baseS || ptrS == 0)
                    goto end;
            }
        }

        _idx.seekg(ptrS + 0x2E);

        for(auto it = 0; it != _header.size(); ++it){
            const uint64_t ptr = _idx.tellg();
            uint32_t ptrSeq = 0;
            _idx.read((char *)&ptrSeq, _header[0].ptrSize);

            if(ptrSeq != 0){
                _idx.seekg(ptrS + 0x16 + ptrSeq);
                char * table = new char[256];

                for(auto sq = 0; sq != 256; ++sq){
                    _idx.read(table + sq, 1);
                    _idx.seekg((uint64_t)_idx.tellg() + 1);
                }

                _header[it].altSeq = table;
            }
            _idx.seekg(ptr + 0x8);
        }
    }

end:_dat.seekg(0x2c);
    _dat.read((char *) &_reclen, sizeof(_reclen));

    if(!_dat.good())
        throw std::runtime_error("Database was destroyed");

    setIndex(0);
}

void openCtree::close() noexcept{
    _header.clear();
    _idx.close();
    _idx.clear();
    _dat.close();
    _dat.clear();
    _index = 0;
    _reclen = 0;
    _navigate = {0, 0, 0, 0};
    _lastKey.clear();
}

int16_t openCtree::indexCount() const{
    if(!isOpen())
        throw std::logic_error("Database isn't open");

    return _header.size();
}

void openCtree::setIndex(const int16_t member){
    if(!isOpen())
        throw std::logic_error("Database isn't open");

    if(_header.size() <= member)
        throw std::logic_error("Incorrect index size");

    _index = member;
    _lastKey.clear();
    _lastKey.resize(_header[_index].key_length);
    _navigate = {0 , 0, 0, 0};
}
int32_t openCtree::size() const{
    if(!isOpen())
        throw std::logic_error("Database isn't open");

    return  _header[_index].active_entries;
}
std::string openCtree::at(const uint32_t index, const bool readDbText){
    if(!isOpen())
        throw std::logic_error("Database isn't open");

    auto iter = _header.begin() + _index;
    if(index >= iter->active_entries)
        throw std::logic_error("Incorrect index value");

    _idx.seekg(iter->bTree_root);

    while(true){//looking for leaf indexes
        uint64_t ptr = _idx.tellg();
        bool lNode;
        uint64_t pointer = 0;
        _idx.seekg(ptr + 17);
        _idx.read((char *) &lNode, sizeof(lNode));

        if(lNode){
            _idx.seekg(ptr);
            break;
        }

        _idx.read((char *) &pointer, iter->ptrSize);

        if(iter->index_type == 0 || iter->index_type == 12){
            _idx.seekg(pointer);
        }
        else
            throw std::runtime_error("Incorrect index compress type");
    }

    for(uint32_t indexCount = 0; indexCount <= index;){//looking for in leafes index
        uint32_t nextPtr, prevPtr;
        uint16_t byteSize, byteCount;
        const uint64_t ptr = _idx.tellg();
        _idx.seekg(ptr);
        _idx.read((char *) &nextPtr, sizeof(nextPtr));
        _idx.seekg(ptr + 4);
        _idx.read((char *) &prevPtr, sizeof(prevPtr));
        _idx.seekg(ptr + 8);
        _idx.read((char *) &byteCount, sizeof(byteCount));

        if(indexCount + byteCount <= index){
            _idx.seekg(nextPtr);
            indexCount += byteCount;
            continue;
        }

        _idx.seekg(ptr + 10);
        _idx.read((char *) &byteSize, sizeof(byteSize));
        _idx.seekg(ptr + 18);

        _navigate._basePtr = ptr;
        _navigate._byteSize = byteSize;
        _navigate._nextHope = nextPtr;

        while(true){
            if(ptr + 18 + byteSize <= _idx.tellg())
                throw std::runtime_error("Incorrect key");

            uint64_t pointer = 0;
            uint64_t di = _idx.tellg();

            if(!iter->dublicate)//leaf dublicate node hasn't poiner in begin
                _idx.read((char *) &pointer, iter->ptrSize);

            if(iter->index_type == 0){//for uncompress indexes
                //there is leaf
                    if(iter->dublicate){
                        pointer = 0;
                        uint8_t tpPtr[4];
                        _idx.seekg(di);
                        _idx.read(_lastKey.data(), iter->key_length);
                        _idx.seekg(di + iter->key_length - 4);
                        _idx.read((char *) &tpPtr, sizeof(tpPtr));

                        for(auto i = 0; i != 4; ++i){
                            ((char *)&pointer)[i] = tpPtr[3 - i];
                        }

                        _idx.seekg(di + iter->key_length);
                    }
                    else{
                        _idx.seekg(di + iter->ptrSize);
                        _idx.read(_lastKey.data(), iter->key_length);
                        _idx.seekg(di + iter->key_length + iter->ptrSize);
                    }

                    if(index == indexCount){
                        _navigate._leafPtr = _idx.tellg();

                        if(readDbText){
                            _dat.seekg(pointer);
                            return gtData();
                        }
                        else
                            return std::string();
                    }
            }
            else if(iter->index_type == 12){//for compress indexes
                uint8_t begCompress, padCompress;
                _idx.read((char *) &begCompress, sizeof(begCompress));
                _idx.read((char *) &padCompress, sizeof(padCompress));
                int16_t mustRead = iter->key_length - begCompress - padCompress + 2;//two 'cause 2 compress bytes

                std::string temp(mustRead - 2, '\0');
                _idx.read(temp.data(), mustRead - 2);

                if(begCompress != 0)//uncompress some bytes from begin
                    temp.insert(0, _lastKey.substr(0, begCompress));
                if(padCompress != 0){//uncompress some bytes from middle
                    if(iter->dublicate)
                        temp.insert(temp.size() - 4, std::string(padCompress, iter->padding));
                    else
                        temp.insert(temp.size(), std::string(padCompress, iter->padding));
                }

                _lastKey = std::move(temp);

                //there is leaf
                    if(iter->dublicate){
                        pointer = 0;
                        uint8_t tpPtr[4];
                        _idx.seekg(di + mustRead - 4);
                        _idx.read((char *) &tpPtr, sizeof(tpPtr));

                        for(auto i = 0; i != 4; ++i){
                            ((char *)&pointer)[i] = tpPtr[3 - i];
                        }

                        _idx.seekg(di + mustRead);
                    }
                    else
                        _idx.seekg(di + mustRead + iter->ptrSize);

                    if(index == indexCount){
                        _navigate._leafPtr = _idx.tellg();

                        if(readDbText){
                            _dat.seekg(pointer);
                            return gtData();
                        }
                        else
                            return std::string();
                    }
                }
            else
                throw std::runtime_error("Incorrect index compress type");

            if(!_idx.good())
                throw std::runtime_error("Database was destroyed");

            ++indexCount;
        }
    }
}
std::string openCtree::at(std::string key, const bool readDbText){//if key is dublicate than input key size == full key size - 4
    if(!isOpen())
        throw std::logic_error("Database isn't open");

    auto iter = _header.begin() + _index;

    if(!iter->dublicate && key.size() != iter->key_length)
        throw std::logic_error("Incoorect key size");
    else if(iter->dublicate && key.size() != iter->key_length - 4)
        throw std::logic_error("Incoorect key size");

    if(iter->dublicate)
        key.append(4, '\0');

    _idx.seekg(iter->bTree_root);
    std::string keyCmp(iter->key_length, '\0');

    while(true){
        uint16_t byteSize;
        bool lNode;
        const uint64_t ptr = _idx.tellg();
        _idx.read((char *) &_navigate._nextHope, sizeof(_navigate._nextHope));

        _idx.seekg(ptr + 10);
        _idx.read((char *) &byteSize, sizeof(byteSize));
        _idx.seekg(ptr + 17);
        _idx.read((char *) &lNode, sizeof(lNode));

        _navigate._byteSize = byteSize;
        _navigate._basePtr = ptr;

        while(true){
            if(ptr + 18 + byteSize <= _idx.tellg())
                throw std::runtime_error("Incorrect key");

            uint64_t pointer = 0;

            if(!(iter->dublicate && lNode))//leaf dublicate node hasn't poiner in begin
                _idx.read((char *) &pointer, iter->ptrSize);

            if(iter->index_type == 0){//for uncompress indexes
                _idx.read(keyCmp.data(), iter->key_length);

                if(!lNode){//there is branch
                    if(keyCmp >= key){
                        _idx.seekg(pointer);
                        break;
                    }
                }
                else{//there is leaf
                    _navigate._leafPtr = _idx.tellg();
                    _lastKey = keyCmp;

                    if(iter->dublicate){
                        pointer = 0;
                        const auto sz = keyCmp.size();

                        for(auto i = 0; i != 4; ++i){
                            ((char *)&pointer)[i] = keyCmp.at(sz - 1 - i);
                            keyCmp.at(sz - 1 - i) = '\0';
                        }
                    }
                    if(keyCmp == key){
                        if(readDbText){
                            _dat.seekg(pointer);
                            return gtData();
                        }
                        else
                            return std::string();
                    }
                }
            }
            else if(iter->index_type == 12){//for compress indexes
                uint8_t begCompress, padCompress;
                _idx.read((char *) &begCompress, sizeof(begCompress));
                _idx.read((char *) &padCompress, sizeof(padCompress));
                int16_t mustRead = iter->key_length - begCompress - padCompress;
                std::string temp(mustRead, '\0');
                _idx.read(temp.data(), mustRead);

                if(begCompress != 0)//uncompress some bytes from begin
                    temp.insert(0, keyCmp.substr(0, begCompress));
                if(padCompress != 0){//uncompress some bytes from middle
                    if(iter->dublicate)
                        temp.insert(temp.size() - 4, std::string(padCompress, iter->padding));
                    else
                        temp.insert(temp.size(), std::string(padCompress, iter->padding));
                }

                _lastKey = temp;
                keyCmp = std::move(temp);

                if(!lNode){//there is branch
                    if(keyCmp >= key){
                        _idx.seekg(pointer);
                        break;
                    }
                }
                else{//there is leaf
                    _navigate._leafPtr = _idx.tellg();

                    if(iter->dublicate){
                        pointer = 0;
                        const auto sz = keyCmp.size();

                        for(auto i = 0; i != 4; ++i){
                            ((char *)&pointer)[i] = keyCmp.at(sz - 1 - i);
                            keyCmp.at(sz - 1 - i) = '\0';
                        }
                    }
                    if(keyCmp == key){
                        if(readDbText){
                            _dat.seekg(pointer);
                            return gtData();
                        }
                        else
                            return std::string();
                    }
                }
            }
            else
                throw std::runtime_error("Incorrect index compress type");

            if(!_idx.good())
                throw std::runtime_error("Database was destroyed");
        }
    }
}
std::string openCtree::front(const bool readDbText){
    return at(0, readDbText);
}
std::string openCtree::back(const bool readDbText){
    return at(size() - 1, readDbText);
}
std::string openCtree::next(const bool readDbText){
    if(!isOpen())
        throw std::logic_error("Database isn't open");

    if(_navigate._leafPtr == 0)
        return at(0);

    auto iter = _header.begin() + _index;
    _idx.seekg(_navigate._leafPtr);

    auto nextLeaf = [&](){
        _idx.seekg(_navigate._nextHope);
        const uint64_t ptr = _navigate._nextHope;

        if(ptr == 0){
            return true;
        }

        _navigate._basePtr = ptr;
        _idx.seekg(ptr);
        _idx.read((char *) &_navigate._nextHope, sizeof(_navigate._nextHope));
        _idx.seekg(ptr + 10);
        _idx.read((char *) &_navigate._byteSize, sizeof(_navigate._byteSize));
        _idx.seekg(ptr + 18);
        return false;
    };

    while(true){
        if(_navigate._basePtr + 18 + _navigate._byteSize <= _idx.tellg() && nextLeaf())
            throw std::logic_error("End of elements");

        uint64_t pointer = 0;
        const uint64_t di = _idx.tellg();

        if(!iter->dublicate)//leaf dublicate node hasn't poiner in begin
            _idx.read((char *) &pointer, iter->ptrSize);

        if(iter->index_type == 0){//for uncompress indexes
            //there is leaf
                if(iter->dublicate){
                    pointer = 0;
                    uint8_t tpPtr[4];
                    _idx.seekg(di);
                    _idx.read(_lastKey.data(), iter->key_length);
                    _idx.seekg(di + iter->key_length - 4);
                    _idx.read((char *) &tpPtr, sizeof(tpPtr));

                    for(auto i = 0; i != 4; ++i){
                        ((char *)&pointer)[i] = tpPtr[3 - i];
                    }

                    _idx.seekg(di + iter->key_length);
                }
                else{
                    _idx.seekg(di + iter->ptrSize);
                    _idx.read(_lastKey.data(), iter->key_length);
                    _idx.seekg(di + iter->key_length + iter->ptrSize);
                }

                _navigate._leafPtr = _idx.tellg();

                if(readDbText){
                    _dat.seekg(pointer);
                    return gtData();
                }
                else
                    return std::string();
        }
        else if(iter->index_type == 12){//for compress indexes
            uint8_t begCompress, padCompress;
            _idx.read((char *) &begCompress, sizeof(begCompress));
            _idx.read((char *) &padCompress, sizeof(padCompress));
            int16_t mustRead = iter->key_length - begCompress - padCompress + 2;//two 'cause 2 compress bytes

            std::string temp(mustRead - 2, '\0');
            _idx.read(temp.data(), mustRead - 2);

            if(begCompress != 0)//uncompress some bytes from begin
                temp.insert(0, _lastKey.substr(0, begCompress));
            if(padCompress != 0){//uncompress some bytes from middle
                if(iter->dublicate)
                    temp.insert(temp.size() - 4, std::string(padCompress, iter->padding));
                else
                    temp.insert(temp.size(), std::string(padCompress, iter->padding));
            }

            _lastKey = std::move(temp);

            //there is leaf
                if(iter->dublicate){
                    pointer = 0;
                    uint8_t tpPtr[4];
                    _idx.seekg(di + mustRead - 4);
                    _idx.read((char *) &tpPtr, sizeof(tpPtr));

                    for(auto i = 0; i != 4; ++i){
                        ((char *)&pointer)[i] = tpPtr[3 - i];
                    }

                    _idx.seekg(di + mustRead);
                }
                else
                    _idx.seekg(di + mustRead + iter->ptrSize);

                _navigate._leafPtr = _idx.tellg();

                if(readDbText){
                    _dat.seekg(pointer);
                    return gtData();
                }
                else
                    return std::string();
            }
        else
            throw std::runtime_error("Incorrect index compress type");

        if(!_idx.good())
            throw std::runtime_error("Database was destroyed");
    }
}
uint16_t openCtree::serviceDataLenght() const{
    if(!isOpen())
        throw std::logic_error("Database isn't open");

    return _reclen;
}

bool openCtree::isOpen() const noexcept{
    return !_header.empty();
}
