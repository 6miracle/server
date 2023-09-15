// 解决粘包问题
#pragma once
#include "ByteArray/ByteArray.h"
#include "pch.h"

namespace server {
class Stream {
public:
    using ptr = std::shared_ptr<Stream>;    

    Stream() = default;
    virtual ~Stream() = default;

    virtual int read(void* buffer, size_t length) = 0;
    virtual int read(ByteArray::ptr ba, size_t length) = 0;
    virtual int readFixSize(void* buffer, size_t length);
    virtual int readFixSize(ByteArray::ptr ba, size_t length);


    virtual int write(void* buffer, size_t length) = 0;
    virtual int write(ByteArray::ptr ba, size_t length) = 0;
    virtual int writeFixSize(const void* buffer, size_t length);
    virtual int writeFixSize(ByteArray::ptr ba, size_t length);

    virtual bool close() = 0;
};
}