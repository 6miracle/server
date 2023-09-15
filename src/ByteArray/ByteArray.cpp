#include "ByteArray.h"
#include "Logger/logger.hpp"
#include <cstddef>
#include <byteswap.h>
#include <cstdint>
#include <cstring>
#include <endian.h>
#include <fstream>
#include <sstream>
namespace server {

template<typename T>
static T byteswap(const T val) {
    if constexpr (sizeof(T) == 2) {
        return bswap_16(val);
    } else if constexpr (sizeof(T) == 4) {
        return bswap_32(val);
    } else if constexpr (sizeof(T) == 8) {
        return bswap_64(val);
    }
}

ByteArray::Node::Node(): ptr(nullptr), next(nullptr), size(0) {}
ByteArray::Node::Node(size_t s): ptr(new char[s]), next(nullptr), size(s) {} 
ByteArray::Node::~Node() { 
    if(ptr) {
        delete []ptr;
    }
}

ByteArray::ByteArray(size_t size):nodeSize_(size), position_(0),
     capacity_(size), size_(0), endian_(BIG_ENDIAN), root_(new Node(size)), cur_(root_) {}

ByteArray::~ByteArray() {
    while(root_ != nullptr) {
        Node* tmp = root_;
        root_ = root_->next;
        delete tmp;
    }
}


void ByteArray::writeFint8(int8_t value) {
    write(&value, 1);
    
}   // 固定长度
void ByteArray::writeFuint8(uint8_t value) {
    LOG_INFO("write %d", value);
    write(&value, 1);
}
void ByteArray::writeFint16(int16_t value) {
    if(endian_ != BYTE_ORDER ){
        value = byteswap(value);
    }
    write(&value, 2);
}
void ByteArray::writeFuint16(uint16_t value) {
    if(endian_ != BYTE_ORDER ){
        value = byteswap(value);
    }
    write(&value, 2);
}
void ByteArray::writeFint32(int32_t value) {
    if(endian_ != BYTE_ORDER ){
        value = byteswap(value);
    }
    write(&value, 4);
}
void ByteArray::writeFuint32(uint32_t value) {
    if(endian_ != BYTE_ORDER ){
        value = byteswap(value);
    }
    write(&value, 4);
}
void ByteArray::writeFint64(int64_t value) {
    if(endian_ != BYTE_ORDER ){
        value = byteswap(value);
    }
    write(&value, 8);
}
void ByteArray::writeFuint64(uint64_t value) {
    if(endian_ != BYTE_ORDER ){
        value = byteswap(value);
    }
    write(&value, 8);
}

static uint32_t encodeZigZag(int32_t value) {
    return (value << 1 ) ^ (value >> 31);
}
static uint64_t encodeZigZag(int64_t value) {
    return (value << 1) ^ (value >> 63);
}
static int32_t decodeZigZag(uint32_t value) {
    return (value >> 1) ^ - (value  & 1);
}
static int64_t decodeZigZag(uint64_t value) {
    return (value >> 1) ^- (value & 1);
}
void ByteArray::writeInt32(int32_t value) {
    writeUint32(encodeZigZag(value));
}
void ByteArray::writeUint32(uint32_t value) {
    uint8_t tmp[5];
    int i = 0;
    while(value >= 0x80) {
        tmp[i++] = (value & 0x7F) | 0x80;
        value >>= 7;
    }
    tmp[i++] = value;
    write(tmp, i);
}
void ByteArray::writeInt64(int64_t value) {
    writeUint64(encodeZigZag(value));
}
void ByteArray::writeUint64(uint64_t value) {
    uint8_t tmp[10];
    int i = 0;
    while(value >= 0x80) {
        tmp[i++] = (value & 0x7F) | 0x80;
        value >>= 7;
    }
    tmp[i++] = value;
    write(tmp, i);
}

void ByteArray::writeFloat(float value) {
    uint32_t v;
    memcpy(&v, &value, 4);
    writeFuint32(v);
}
void ByteArray::writeDouble(double value) {
    uint64_t v;
    memcpy(&v, &value, 8);
    writeFuint64(v);
}
void ByteArray::writeStringF16(const std::string& value) {
    LOG_INFO("str = %s, size = %ld", value.c_str(), value.size());
    writeFuint16(value.size());
    write(value.c_str(), value.size());
}
void ByteArray::writeStringF32(const std::string& value) {
    writeFuint32(value.size());
    write(value.c_str(), value.size());
}
void ByteArray::writeStringF64(const std::string& value) {
    writeFuint64(value.size());
    write(value.c_str(), value.size());
}
void ByteArray::writeString64(const std::string& value) {
    writeUint64(value.size());
    write(value.c_str(), value.size());
}
void ByteArray::writeString(const std::string& value) {
    write(value.c_str(), value.size());
}

// read
int8_t ByteArray::readFint8() {
    int8_t val;
    read(&val, 1);
    return val;
}
uint8_t ByteArray::readFuint8() {
    uint8_t val;
    read(&val, 1);
    return val;
}
int16_t ByteArray::readFint16() {
    int16_t val;
    read(&val, 2);
    if(endian_ != BYTE_ORDER) {
        val = byteswap(val);
    }
    return val;
}
uint16_t ByteArray::readFuint16() {
    uint16_t val;
    read(&val, 2);
    if(endian_ != BYTE_ORDER) {
        val = byteswap(val);
    }
    return val;
}
int32_t ByteArray::readFint32() {
    int32_t val;
    read(&val, 4);
    if(endian_ != BYTE_ORDER) {
        val = byteswap(val);
    }
    return val;
}
uint32_t ByteArray::readFuint32() {
    uint32_t val;
    read(&val, 4);
    if(endian_ != BYTE_ORDER) {
        val = byteswap(val);
    }
    return val;
}
int64_t ByteArray::readFint64() {
    int64_t val;
    read(&val, 8);
    if(endian_ != BYTE_ORDER) {
        val = byteswap(val);
    }
    return val;
}
uint64_t ByteArray::readFuint64() {
    uint64_t val;
    read(&val, 8);
    if(endian_ != BYTE_ORDER) {
        val = byteswap(val);
    }
    return val;
}

int32_t ByteArray::readInt32() {
    return decodeZigZag(readUint32());
}
uint32_t ByteArray::readUint32() {
    uint32_t val;
    uint8_t num;
    for(int i = 0; i < 32; i += 7) {
        num = readFuint8();
        if(num < 0x80) {
            val |= (uint32_t)num << i;
            break;
        }
        val |= (uint32_t)(num & 0x7F) << i;
    }
    return val;
}
int64_t ByteArray::readInt64() {
    return decodeZigZag(readUint64());
}
uint64_t ByteArray::readUint64() {
    uint64_t val;
    uint8_t num;
    for(int i = 0; i < 64; i += 7) {
        num = readFuint8();
        if(num < 0x80) {
            val |= (uint64_t)num << i;
            break;
        }
        val |= (uint64_t)(num & 0x7F) << i;
    }
    return val;
}

float ByteArray::readFloat() {
    uint32_t result = readFuint32();
    float val;
    memcpy(&val, &result, 4);
    return val;
}
double ByteArray::readDouble() {
    uint64_t result = readFuint64();
    double val;
    memcpy(&val, &result, 8);
    return val;
}

std::string ByteArray::readStringF16() {
    uint16_t size = readFuint16();
    LOG_INFO("read size = %d", size);
    std::string str;
    str.resize(size);
    read(&str[0], size);
    return str;
}
std::string ByteArray::readStringF32() {
    uint32_t size = readFuint32();

    std::string str;
    str.resize(size);
    read(&str[0], size);
    return str;
}
std::string ByteArray::readStringF64() {
    uint64_t size = readFuint64();
    std::string str;
    str.resize(size);
    read(&str[0], size);
    return str;
}
std::string ByteArray::readString64() {
    uint64_t size = readUint64();
    std::string str;
    str.resize(size);
    read(&str[0], size);
    return str;
}

void ByteArray::write(const void* buf, size_t size) {
    if(size == 0) { return ; } 
    size_t nPos = position_ % nodeSize_;
    size_t nCap = cur_->size - nPos;
    size_t bPos = 0;
    LOG_INFO("nPos = %ld, nCap = %ld", nPos, nCap);
    while(size > 0) {
        if(nCap >= size) {
            memcpy(cur_->ptr + nPos, (char*)buf + bPos, size);
            bPos += size;
            size = 0;
            position_ += size;
            size_ += size;
        } else {
            memcpy(cur_->ptr + nPos, (char*)buf + bPos, nCap);
            size -= nCap;
            size_ += nCap;
            bPos += nCap;
            position_ += nCap;
            if(!cur_->next) {
                cur_->next = new Node(nodeSize_);
            }
            cur_ = cur_->next;
            nPos = 0;
            nCap = nodeSize_;
        }
    }
    LOG_INFO("nPos = %ld, nCap = %ld", nPos, nCap);
}
void ByteArray::read(void* buf, size_t size) {
    if(size == 0) return ;
    size_t nPos = position_ % nodeSize_;
    size_t nCap = cur_->size - nPos;
    size_t bPos = 0;

    while(size > 0) {
        if(nCap >= size) {
            memcpy((char*)buf + bPos, cur_->ptr + nPos, size);
            position_ += size;
            size = 0;
        } else {
            memcpy((char*)buf + bPos, cur_->ptr + nPos, nCap);
            position_ += nCap;
            if(cur_->next == NULL) break;
            else{ cur_ = cur_ ->next; }
            size -= nCap;
            nPos  = 0;
            nCap = nodeSize_;
            bPos += nCap;
        }
    }
}
ByteArray::Node* ByteArray::GetNode(size_t position) const {
    size_t num = position / nodeSize_;
    Node* cur = root_;
    while(num--) {
        if(cur->next == NULL) return nullptr;
        cur = cur->next;
    }
    return cur;
}
void ByteArray::read(void* buf, size_t size, size_t position) const{
    if(size == 0) return ;
    Node* cur = GetNode(position);
    if(cur == nullptr) return ;
    size_t nPos = position % nodeSize_;
    size_t nCap = cur->size - nPos;
    size_t bPos = 0;

    while(size > 0) {
        if(nCap >= size) {
            memcpy((char*)buf + bPos, cur->ptr + nPos, size);
            size = 0;
        } else {
            memcpy((char*)buf + bPos, cur->ptr + nPos, nCap);
            position += nCap;
            if(cur->next == NULL) break;
            else{ cur = cur ->next; }
            size -= nCap;
            nPos  = 0;
            nCap = nodeSize_;
            bPos += nCap;
        }
    }
}


uint64_t ByteArray::getReadBuffers(std::vector<iovec>& buffers, uint64_t size) const {
    if(size == 0) return 0;
    size_t nPos = position_ % nodeSize_;
    size_t nCap = cur_->size - nPos;
    struct iovec iov;
    Node* cur = cur_;
    uint64_t len = size;

    while(size > 0) {
        if(nCap >= size) {
            iov.iov_base = cur->ptr + nPos;
            iov.iov_len = size;
            size = 0;
        } else {
            iov.iov_base = cur_->ptr + nPos;
            iov.iov_len = nCap;
            if(cur->next == NULL) break;
            else{ cur = cur ->next; }
            size -= nCap;
            nPos  = 0;
            nCap = nodeSize_;
        }
        buffers.push_back(iov);
    }
    return len;
}
uint64_t ByteArray::getWriteBuffers(std::vector<iovec>& buffers, uint64_t size) {
    if(size == 0) { return 0; } 
    
    size_t nPos = position_ % nodeSize_;
    size_t nCap = cur_->size - nPos;
    struct iovec iov;
    Node* cur = cur_;
    uint64_t len = size;
    while(size > 0) {
        if(nCap >= size) {
            iov.iov_base = cur_->ptr + nPos;
            iov.iov_len = size;
            size = 0;
        } else {
            iov.iov_base = cur_ + nPos;
            iov.iov_len = nCap;
            size -= nCap;
            position_ += nCap;
            if(!cur->next) {
                cur->next = new Node(nodeSize_);
            }
            cur = cur->next;
            nPos = 0;
            nCap = nodeSize_;
        }
        buffers.push_back(iov);
    }
    return len;
}

// 设置ByteArray当前位置
void ByteArray::setPosition(size_t n) {
    if(n > capacity_) {
        LOG_ERROR("set_position out of range");
        return ;
    }
    position_ = n;
    if(n > size_) {
        size_ = n;
    }
    cur_ = GetNode(n);
}
// 把ByteArray的数据写入到文件中
void ByteArray::writeFile(const std::string& name) const {
    std::ofstream os;
    os.open(name, std::ios::trunc | std::ios::binary);
    if(!os) { 
        LOG_ERROR("open file error");
        return ;
    } 

    size_t read_size = GetReadSize();
    size_t nPos = position_ % nodeSize_;
    size_t nCap = cur_->size - nPos;
    Node* cur = cur_;
    while(read_size > 0) {
        // size_t len = read_size >= nodeSize_ ? nodeSize_ : read_size;
        size_t len = read_size > nCap ? nCap : read_size;
        os.write(cur->ptr + nPos, len);
        read_size -= len;
        cur = cur->next;
        nCap = nodeSize_;
        nPos = 0;
    }

}
void ByteArray::readFromFile(const std::string& name) {
    std::ifstream is;
    is.open(name, std::ios::binary);
    if(!is) {
        LOG_ERROR("open file error");
        return ;
    }

    char tmp[nodeSize_];
    while(!is.eof()) {
        is.read(tmp, nodeSize_);
        write(tmp, is.gcount());
    }
}

std::string ByteArray::ToString() const {
    std::string str;
    str.resize(GetReadSize());
    if(str.empty()) { return str; }
    read(&str[0], str.size(), position_); 
    return str;
}
}