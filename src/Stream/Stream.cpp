#include "Stream.h"

namespace server {
int Stream::readFixSize(void* buffer, size_t length) {
    size_t offset = 0;
    size_t l = length;
    while(length > 0) {
        size_t len = read((char*)buffer + offset, length);
        if(len <= 0) {
            LOG_ERROR("read error");
            return len;
        }
        offset += len;
        length -= len;
    }
    return l;
}
int Stream::readFixSize(ByteArray::ptr ba, size_t length) {
    size_t offset = length;
    while(length > 0) {
        size_t len = read(ba, length);
        if(len <= 0) {
            LOG_ERROR("read error");
            return len;
        }
        length -= len;
    }
    return offset;
}

int Stream::writeFixSize(const void* buffer, size_t length) {
    size_t offset = 0;
    size_t l = length;
    while(length > 0) {
        size_t len = write((char*)buffer + offset, length);
        if(len <= 0) {
            LOG_ERROR("read error");
            return len;
        }
        offset += len;
        length -= len;
    }
    return l;
}
int Stream::writeFixSize(ByteArray::ptr ba, size_t length) {
    size_t offset = length;
    while(length > 0) {
        size_t len = write(ba, length);
        if(len <= 0) {
            LOG_ERROR("read error");
            return len;
        }
        length -= len;
    }
    return offset;
}
}