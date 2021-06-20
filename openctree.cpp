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
uint8_t openCtree::pointerSize() const{
    if(!isOpen())
        throw std::logic_error("Database isn't open");

    return _header[_index].ptrSize;
}
uint16_t openCtree::keyLenght() const{
    if(!isOpen())
        throw std::logic_error("Database isn't open");

    return _header[_index].key_length;
}
bool openCtree::isAlternateSequence() const{
    if(!isOpen())
        throw std::logic_error("Database isn't open");

    if(_header[_index].altSeq == nullptr)
        return false;

    return true;
}
std::string openCtree::encodeKey(const std::string & key) const{
    //encode text to key value (hello -> VKbbj)
    if(!isAlternateSequence())
        throw std::logic_error("Member hasn't an alternate sequence");

    std::string str;
    str.reserve(key.size());

    const auto ptr = _header[_index].altSeq;

    for(auto & it : key)
        str.push_back(ptr[(uint8_t)it]);

    return str;
}
std::string openCtree::decodeKey(const std::string & key) const{
    //decode key value to text (VKbbj -> hello)
    if(!isAlternateSequence())
        throw std::logic_error("Member hasn't an alternate sequence");

    std::string str;
    str.reserve(key.size());

    const auto ptr = _header[_index].altSeq;

    for(auto & it : key){
        for(auto i = 0; i != 255; ++i){
            if(it == ptr[i])
                str.push_back((char) i);
        }
    }

    return str;
}
std::string openCtree::gtData(const uint64_t datPos){
    uint8_t sig[2];
    _dat.seekg(datPos);
    _dat.read((char *) &sig, 2);

    constexpr const uint8_t sg[] = {250, 250};
    if(memcmp(sig, sg, 2) != 0)
        throw std::runtime_error("Incorrect signature");

    uint32_t dtSize = 0;
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
openCtree::openCtree(const openCtree & copy){
    _header = copy._header;
    _index = copy._index;
    _reclen = copy._reclen;
    _lastKey = copy._lastKey;
    _lastValuePos = copy._lastValuePos;
    _lastPosition = copy._lastPosition;
    _navigate = copy._navigate;
    _fileName = copy._fileName;
    _leaf = copy._leaf;

    _idx.open(_fileName + ".idx", std::ios::in | std::ios::binary);
    _dat.open(_fileName + ".dat", std::ios::in | std::ios::binary);
    _idx.sync_with_stdio(false);
    _dat.sync_with_stdio(false);

    if(!_idx.is_open() || !_dat.is_open())
        throw std::runtime_error("Database isn't open");
}
openCtree & openCtree::operator=(const openCtree & copy){
    _header = copy._header;
    _index = copy._index;
    _reclen = copy._reclen;
    _lastKey = copy._lastKey;
    _lastValuePos = copy._lastValuePos;
    _lastPosition = copy._lastPosition;
    _navigate = copy._navigate;
    _fileName = copy._fileName;
    _leaf = copy._leaf;

    _idx.open(_fileName + ".idx", std::ios::in | std::ios::binary);
    _dat.open(_fileName + ".dat", std::ios::in | std::ios::binary);
    _idx.sync_with_stdio(false);
    _dat.sync_with_stdio(false);

    if(!_idx.is_open() || !_dat.is_open())
        throw std::runtime_error("Database isn't open");

    return *this;
}
void openCtree::open(const std::string & filename){
    if(isOpen())
        close();

    _fileName = filename;

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

        for(auto & it : _header){
            const uint64_t ptr = _idx.tellg();
            uint32_t ptrSeq = 0;
            _idx.read((char *)&ptrSeq, _header[0].ptrSize);

            if(ptrSeq != 0){
                _idx.seekg(ptrS + 0x16 + ptrSeq);
                auto table = new char[256];

                for(auto sq = 0; sq != 256; ++sq){
                    _idx.read(table + sq, 1);
                    _idx.seekg((uint64_t)_idx.tellg() + 1);
                }

                it.altSeq.reset(table);
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
    _lastValuePos = 0;
    _lastPosition = std::numeric_limits<uint64_t>::max();
    _fileName.clear();
    _leaf.clear();
}

uint16_t openCtree::indexCount() const{
    if(!isOpen())
        throw std::logic_error("Database isn't open");

    return _header.size();
}

void openCtree::setIndex(const uint16_t member){
    if(!isOpen())
        throw std::logic_error("Database isn't open");

    if(_header.size() <= member)
        throw std::logic_error("Incorrect index size");

    _index = member;
    _lastKey.clear();
    _lastKey.resize(_header[_index].key_length);
    _navigate = {0 , 0, 0, 0};
    _lastValuePos = 0;
    _lastPosition = std::numeric_limits<uint64_t>::max();
    _leaf.clear();

    writeLeafs();//save leafs ptr and index pos into map
}
void openCtree::writeLeafs(){
    auto iter = _header.begin() + _index;

    if(iter->bTree_root == 0)
        return;

    _idx.seekg(iter->bTree_root);

    while(true){//looking for leaf indexes
        const uint64_t ptr = _idx.tellg();
        bool lNode = false;
        uint32_t pointer = 0;
        _idx.seekg(ptr + 17);
        _idx.read((char *) &lNode, sizeof(lNode));

        if(lNode){
            _idx.seekg(ptr);
            break;
        }

        _idx.read((char *) &pointer, iter->ptrSize);

        if(iter->index_type == 0 || iter->index_type == 12)
            _idx.seekg(pointer);
        else
            throw std::runtime_error("Incorrect index compress type");

        if(!_idx.good())
            throw std::runtime_error("Database was destroyed");
    }

    int32_t indexCount = 0;

    while(true){//writing base leafs
        uint32_t nextPtr = 0;
        uint16_t byteCount = 0;
        const uint64_t ptr = _idx.tellg();

        if(ptr == 0)
            throw std::logic_error("This position doesn't exist");

        _idx.seekg(ptr);
        _idx.read((char *) &nextPtr, sizeof(nextPtr));

        _idx.seekg(ptr + iter->ptrSize * 2);
        _idx.read((char *) &byteCount, sizeof(byteCount));

        _leaf[indexCount] = ptr;

        if(nextPtr == 0)
            break;

        _idx.seekg(nextPtr);
        indexCount += byteCount;

        if(!_idx.good())
            throw std::runtime_error("Database was destroyed");
    }
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
    if(iter->active_entries < 0 || index >= (uint32_t) iter->active_entries)
        throw std::logic_error("Incorrect index value");

    if(index == _lastPosition)
        return currentValue();

    if(iter->bTree_root == 0)
        throw std::runtime_error("Index is empty");

    auto iterLeaf = _leaf.lower_bound(index);

    if(iterLeaf == _leaf.cend())
        throw std::runtime_error("Database was destroed");

    _idx.seekg(iterLeaf->second);
    uint64_t indexCount = iterLeaf->first;

    const uint64_t ptr = _idx.tellg();
    uint16_t byteSize = 0;
    uint32_t nextPtr = 0;
    _idx.read((char *) &nextPtr, sizeof(nextPtr));
    _idx.seekg(ptr + 10);
    _idx.read((char *) &byteSize, sizeof(byteSize));
    _idx.seekg(ptr + 18);

    _navigate._basePtr = ptr;
    _navigate._byteSize = byteSize;
    _navigate._nextHope = nextPtr;

    while(true){//finding value
        {
            const auto pointerTemp = _idx.tellg();

            if(pointerTemp < 0 || ptr + 18 + byteSize <= (uint64_t) pointerTemp)
                throw std::runtime_error("Incorrect key");
        }

        uint64_t pointer = 0;

        if(!iter->dublicate)//leaf dublicate node hasn't poiner in begin
            _idx.read((char *) &pointer, iter->ptrSize);

        if(iter->index_type == 0){//for uncompress indexes
            //there is leaf
                if(iter->dublicate){
                    pointer = 0;
                    _idx.read(_lastKey.data(), iter->key_length);
                    std::copy(_lastKey.crbegin(), _lastKey.crbegin() + iter->ptrSize, (uint8_t *) &pointer);
                }
                else
                    _idx.read(_lastKey.data(), iter->key_length);

                if(index == indexCount){
                    _lastValuePos = pointer;
                    _navigate._leafPtr = _idx.tellg();
                    _lastPosition = index;

                    return readOrNot(readDbText, pointer);
                }
        }
        else if(iter->index_type == 12){//for compress indexes
            _lastKey = uncompressString(_lastKey);

            //there is leaf
                if(iter->dublicate){
                    pointer = 0;
                    std::copy(_lastKey.crbegin(), _lastKey.crbegin() + iter->ptrSize, (uint8_t *) &pointer);
                }

                if(index == indexCount){
                    _lastValuePos = pointer;
                    _navigate._leafPtr = _idx.tellg();
                    _lastPosition = index;

                    return readOrNot(readDbText, pointer);
                }
            }
        else
            throw std::runtime_error("Incorrect index compress type");

        if(!_idx.good())
            throw std::runtime_error("Database was destroyed");

        ++indexCount;
    }
}
std::string openCtree::at(std::string key, const bool readDbText){//if key is dublicate than input key size == full key size - 4
    return std::get<std::string>(commonAtKey(key, true, readDbText));
}
bool openCtree::haveKey(std::string key){
    return std::get<bool>(commonAtKey(key, false, false));
}
std::variant<bool, std::string> openCtree::commonAtKey(std::string key, const bool savePos, const bool readDbText){
    if(!isOpen())
        throw std::logic_error("Database isn't open");

    auto iter = _header.begin() + _index;

    if(!iter->dublicate && key.size() != iter->key_length)
        throw std::logic_error("Incoorect key size");
    else if(iter->dublicate && key.size() != iter->key_length - iter->ptrSize)
        throw std::logic_error("Incoorect key size");

    if(iter->dublicate)
        key.append(iter->ptrSize, '\0');
    else if(key == _lastKey)
        return currentValue();

    if(iter->bTree_root == 0)
        throw std::runtime_error("Index is empty");

    _idx.seekg(iter->bTree_root);
    std::string keyCmp(iter->key_length, '\0');

    while(true){
        uint16_t byteSize = 0;
        bool lNode = false;
        const uint64_t ptr = _idx.tellg();
        _idx.read((char *) &_navigate._nextHope, sizeof(_navigate._nextHope));

        _idx.seekg(ptr + 10);
        _idx.read((char *) &byteSize, sizeof(byteSize));
        _idx.seekg(ptr + 17);
        _idx.read((char *) &lNode, sizeof(lNode));

        if(savePos){
            _navigate._byteSize = byteSize;
            _navigate._basePtr = ptr;
        }

        while(true){
            {
                const auto pointerTemp = _idx.tellg();

                if(pointerTemp < 0 || ptr + 18 + byteSize <= (uint64_t) pointerTemp){
                    if(savePos)
                        throw std::runtime_error("Incorrect key");

                    return false;
                }
            }

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
                    if(iter->dublicate){
                        pointer = 0;
                        std::copy(keyCmp.crbegin(), keyCmp.crbegin() + iter->ptrSize, (char *) &pointer);
                        std::fill(keyCmp.rbegin(), keyCmp.rbegin() + iter->ptrSize, 0);
                    }

                    if(savePos){
                        _lastValuePos = pointer;
                        _navigate._leafPtr = _idx.tellg();
                    }

                    if(keyCmp == key){
                        if(!savePos)
                            return true;
                        else{
                            _lastKey = keyCmp;

                            if(iter->dublicate)
                                std::copy((char *) &pointer, ((char *) &pointer) + iter->ptrSize, _lastKey.rbegin());
                        }

                        _lastPosition = std::numeric_limits<uint64_t>::max();
                        return readOrNot(readDbText, pointer);
                    }
                }
            }
            else if(iter->index_type == 12){//for compress indexes
                keyCmp = uncompressString(keyCmp);

                if(!lNode){//there is branch
                    if(keyCmp >= key){
                        _idx.seekg(pointer);
                        break;
                    }
                }
                else{//there is leaf
                    if(iter->dublicate){
                        pointer = 0;
                        std::copy(keyCmp.crbegin(), keyCmp.crbegin() + iter->ptrSize, (char *) &pointer);
                        std::fill(keyCmp.rbegin(), keyCmp.rbegin() + iter->ptrSize, 0);
                    }

                    if(savePos){
                        _lastValuePos = pointer;
                        _navigate._leafPtr = _idx.tellg();
                    }

                    if(keyCmp == key){
                        if(!savePos)
                            return true;
                        else{
                            _lastKey = keyCmp;

                            if(iter->dublicate)
                                std::copy((char *) &pointer, ((char *) &pointer) + iter->ptrSize, _lastKey.rbegin());
                        }

                        _lastPosition = std::numeric_limits<uint64_t>::max();
                        return readOrNot(readDbText, pointer);
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
    if(!isOpen())
        throw std::logic_error("Database isn't open");

    auto iter = _header.begin() + _index;

    if(iter->bTree_root == 0)
        throw std::runtime_error("Index is empty");

    std::string key(iter->key_length, '\xFF');
    _idx.seekg(iter->bTree_root);
    std::string keyCmp(iter->key_length, '\0');

    while(true){
        uint16_t byteSize = 0;
        bool lNode = false;
        const uint64_t ptr = _idx.tellg();
        _idx.read((char *) &_navigate._nextHope, sizeof(_navigate._nextHope));

        _idx.seekg(ptr + 10);
        _idx.read((char *) &byteSize, sizeof(byteSize));
        _idx.seekg(ptr + 17);
        _idx.read((char *) &lNode, sizeof(lNode));

        _navigate._byteSize = byteSize;
        _navigate._basePtr = ptr;

        while(true){
            {
                const auto pointerTemp = _idx.tellg();

                if(pointerTemp < 0 || ptr + 18 + byteSize <= (uint64_t) pointerTemp)
                    throw std::runtime_error("Incorrect key");
            }

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
                    _lastKey = keyCmp;

                    if(iter->dublicate){
                        pointer = 0;
                        std::copy(keyCmp.crbegin(), keyCmp.crbegin() + iter->ptrSize, (char *) &pointer);
                    }

                    _lastValuePos = pointer;
                    _navigate._leafPtr = _idx.tellg();

                    if(ptr + 18 + byteSize == _navigate._leafPtr){
                        _lastPosition = size() - 1;
                        return readOrNot(readDbText, pointer);
                    }
                }
            }
            else if(iter->index_type == 12){//for compress indexes
                _lastKey = uncompressString(keyCmp);
                keyCmp = _lastKey;

                if(!lNode){//there is branch
                    if(keyCmp >= key){
                        _idx.seekg(pointer);
                        break;
                    }
                }
                else{//there is leaf
                    if(iter->dublicate){
                        pointer = 0;
                        std::copy(keyCmp.crbegin(), keyCmp.crbegin() + iter->ptrSize, (char *) &pointer);
                    }

                    _navigate._leafPtr = _idx.tellg();
                    _lastValuePos = pointer;

                    if(ptr + 18 + byteSize == _navigate._leafPtr){
                        _lastPosition = size() - 1;
                        return readOrNot(readDbText, pointer);
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
std::string openCtree::next(const bool readDbText){
    if(!isOpen())
        throw std::logic_error("Database isn't open");

    if(_navigate._leafPtr == 0)
        return front(readDbText);

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
        {
            const auto pointerTemp = _idx.tellg();

            if(pointerTemp < 0 ||
                    (_navigate._basePtr + 18 + _navigate._byteSize <=
                    (uint64_t) pointerTemp && nextLeaf()))
                throw std::logic_error("End of elements");
        }

        uint64_t pointer = 0;

        if(!iter->dublicate)//leaf dublicate node hasn't poiner in begin
            _idx.read((char *) &pointer, iter->ptrSize);

        if(iter->index_type == 0){//for uncompress indexes
            //there is leaf
                if(iter->dublicate){
                    pointer = 0;
                    _idx.read(_lastKey.data(), iter->key_length);
                    std::copy(_lastKey.crbegin(), _lastKey.crbegin() + iter->ptrSize, (uint8_t *) &pointer);
                }
                else
                    _idx.read(_lastKey.data(), iter->key_length);

                _navigate._leafPtr = _idx.tellg();
                _lastValuePos = pointer;

                if(_lastPosition != std::numeric_limits<uint64_t>::max())
                    ++_lastPosition;

                return readOrNot(readDbText, pointer);
        }
        else if(iter->index_type == 12){//for compress indexes
            _lastKey = uncompressString(_lastKey);

            //there is leaf
            if(iter->dublicate){
                pointer = 0;
                std::copy(_lastKey.crbegin(), _lastKey.crbegin() + iter->ptrSize, (uint8_t *) &pointer);
            }

            if(_lastPosition != std::numeric_limits<uint64_t>::max())
                ++_lastPosition;

            _navigate._leafPtr = _idx.tellg();
            _lastValuePos = pointer;

            return readOrNot(readDbText, pointer);
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
std::string openCtree::currentValue(){
    if(!isOpen())
        throw std::logic_error("Database isn't open");

    if(_lastValuePos == 0 || _lastKey.empty())
        throw std::logic_error("Position is not open");

    return gtData(_lastValuePos);
}
std::string openCtree::readOrNot(const bool readDbText, const uint64_t pointer){
    if(readDbText)
        return gtData(pointer);

    return std::string();
}
uint64_t openCtree::currentPosition(){
    if(!isOpen())
        throw std::logic_error("Database isn't open");
    if(_navigate._leafPtr == 0)
        throw std::logic_error("Position is not open");
    if(_lastPosition != std::numeric_limits<uint64_t>::max())
        return _lastPosition;

    auto iter = _header.begin() + _index;

    if(iter->bTree_root == 0)
        throw std::runtime_error("Index is empty");

    _idx.seekg(iter->bTree_root);

    while(true){//go until leaf index
        const uint64_t ptr = _idx.tellg();
        bool lNode = false;
        uint64_t pointer = 0;
        _idx.seekg(ptr + 17);
        _idx.read((char *) &lNode, sizeof(lNode));

        if(lNode){
            _idx.seekg(ptr);
            break;
        }

        _idx.read((char *) &pointer, iter->ptrSize);

        if(iter->index_type == 0 || iter->index_type == 12)
            _idx.seekg(pointer);
        else
            throw std::runtime_error("Incorrect index compress type");
    }

    uint64_t byteCount = 0;

    while(true){//finding a leaf with equal base point
        const uint64_t ptr = _idx.tellg();

        if(ptr == _navigate._basePtr)
            break;
        else if(ptr == 0)
            throw std::logic_error("This position doesn't exist");

        uint32_t nextPtr = 0;
        uint16_t tempByteCount = 0;

        _idx.read((char *) &nextPtr, sizeof(nextPtr));
        _idx.seekg(ptr + iter->ptrSize * 2);
        _idx.read((char *) &tempByteCount, sizeof(tempByteCount));

        byteCount += tempByteCount;
        _idx.seekg(nextPtr);

        if(!_idx.good())
            throw std::runtime_error("Database was destroyed");
    }

    const uint64_t ptr = _idx.tellg();
    uint16_t byteSize = 0;
    _idx.seekg(ptr + 10);
    _idx.read((char *) &byteSize, sizeof(byteSize));
    _idx.seekg(ptr + 18);

    while(true){//finding pointer with equal key
        {
            const auto pointerTemp = _idx.tellg();

            if(pointerTemp < 0 || ptr + 18 + byteSize <= (uint64_t) pointerTemp)
                throw std::runtime_error("Incorrect key");
        }

        uint64_t pointer = 0;
        std::string tempKey(iter->key_length, '\0');

        if(!iter->dublicate)//leaf dublicate node hasn't poiner in begin
            _idx.read((char *) &pointer, iter->ptrSize);

        if(iter->index_type == 0){//for uncompress indexes
            //there is leaf
                if(iter->dublicate){
                    pointer = 0;
                    _idx.read(tempKey.data(), iter->key_length);
                    std::copy(tempKey.crbegin(), tempKey.crbegin() + iter->ptrSize, (uint8_t *) &pointer);
                }
                else
                    _idx.read(tempKey.data(), iter->key_length);

                if(tempKey == _lastKey){
                    _lastValuePos = pointer;
                    _navigate._leafPtr = _idx.tellg();
                    _lastPosition = byteCount;
                    return byteCount;
                }
        }
        else if(iter->index_type == 12){//for compress indexes
            tempKey = uncompressString(tempKey);

            //there is leaf
                if(iter->dublicate){
                    pointer = 0;
                    std::copy(tempKey.crbegin(), tempKey.crbegin() + iter->ptrSize, (uint8_t *) &pointer);
                }

                if(tempKey == _lastKey){
                    _lastValuePos = pointer;
                    _navigate._leafPtr = _idx.tellg();
                    _lastPosition = byteCount;
                    return byteCount;
                }
            }
        else
            throw std::runtime_error("Incorrect index compress type");

        if(!_idx.good())
            throw std::runtime_error("Database was destroyed");

        ++byteCount;
    }
}
std::string openCtree::uncompressString(const std::string & keyCmp){
    auto iter = _header.begin() + _index;
    uint8_t begCompress = 0, padCompress = 0;
    _idx.read((char *) &begCompress, sizeof(begCompress));
    _idx.read((char *) &padCompress, sizeof(padCompress));
    int16_t mustRead = iter->key_length - begCompress - padCompress;
    std::string temp(mustRead, '\0');
    _idx.read(temp.data(), mustRead);

    if(begCompress != 0)//uncompress some bytes from begin
        temp.insert(0, keyCmp.substr(0, begCompress));
    if(padCompress != 0){//uncompress some bytes from middle
        if(iter->dublicate)
            temp.insert(temp.size() - iter->ptrSize, std::string(padCompress, iter->padding));
        else
            temp.insert(temp.size(), std::string(padCompress, iter->padding));
    }

    return temp;
}
bool openCtree::isIteratorSet() const noexcept{
    if(isOpen() && _navigate._leafPtr != 0)
        return true;

    return false;
}
