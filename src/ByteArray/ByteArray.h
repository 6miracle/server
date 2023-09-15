#pragma once

#include "pch.h"
#include <endian.h>

namespace server {

class ByteArray {
public:
    using ptr = std::shared_ptr<ByteArray>;

    ByteArray(size_t size = 4096);
    ~ByteArray();

    struct Node {
        Node();
        Node(size_t s);
        ~Node();
        char* ptr;
        Node* next;
        size_t size;
    };
    // write
    void writeFint8( int8_t value);   // 固定长度
    void writeFuint8(uint8_t value);
    void writeFint16(int16_t value);
    void writeFuint16(uint16_t value);
    void writeFint32(int32_t value);
    void writeFuint32(uint32_t value);
    void writeFint64(int64_t value);
    void writeFuint64(uint64_t value);

    void writeInt32(int32_t value);
    void writeUint32(uint32_t value);
    void writeInt64(int64_t value);
    void writeUint64(uint64_t value);

    void writeFloat(const float value);
    void writeDouble(const double value);

    void writeStringF16(const std::string& value);
    void writeStringF32(const std::string& value);
    void writeStringF64(const std::string& value);
    void writeString64(const std::string& value);
    void writeString(const std::string& value);

    // read
    int8_t readFint8();
    uint8_t readFuint8();
    int16_t readFint16();
    uint16_t readFuint16();
    int32_t readFint32();
    uint32_t readFuint32();
    int64_t readFint64();
    uint64_t readFuint64();

    int32_t readInt32();
    uint32_t readUint32();
    int64_t readInt64();
    uint64_t readUint64();

    float readFloat();
    double readDouble();
    std::string readStringF16();
    std::string readStringF32();
    std::string readStringF64();
    std::string readString64();
    // std::string readString();

    // 设置ByteArray当前位置
    void setPosition(size_t n);
    // 把ByteArray的数据写入到文件中
    void writeFile(const std::string& name) const;
    void readFromFile(const std::string& name);

    void write(const void* buf, size_t size);
    void read(void* buf, size_t size);
    void read(void* buf, size_t size, size_t position) const;

    uint64_t getReadBuffers(std::vector<iovec>& buffers, uint64_t len) const;
    uint64_t getWriteBuffers(std::vector<iovec>& buffers, uint64_t len);

    Node* GetNode(size_t position) const;

    size_t GetReadSize() const { return size_ - position_; }
    size_t getPosition() const { return position_; }
    std::string ToString() const;
private:
    size_t nodeSize_;
    size_t position_;    // 位置
    size_t capacity_;    // 容量
    size_t size_;        // 实际大小
    int endian_;       // 字节序  
    Node* root_;
    Node* cur_;
};  
}