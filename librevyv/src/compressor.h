#ifndef REVYV_COMPRESSOR_H
#define REVYV_COMPRESSOR_H

#include <lzo/lzo1x.h>
#include <sstream>
#include <unistd.h>

class FailedToCompressDataError : public std::runtime_error {
public:
    FailedToCompressDataError()
        : std::runtime_error("Failed to compress data")
    {
    }
};

class Compressor {
public:
    Compressor(unsigned char* data, uint64_t size)
    {
        _data = std::shared_ptr<unsigned char[]>(new unsigned char[size]);
        _wrkmem = malloc(LZO1X_1_MEM_COMPRESS);
        if (lzo1x_1_compress(data, size, _data.get(), &_size, _wrkmem) != LZO_E_OK || _size >= size) {
            throw FailedToCompressDataError();
        }
    }

    ~Compressor() { free(_wrkmem); }

    [[nodiscard]] const std::shared_ptr<unsigned char[]>& getData() const
    {
        return _data;
    }

    [[nodiscard]] size_t getSize() const { return _size; }

private:
    std::shared_ptr<unsigned char[]> _data;
    size_t _size {};
    void* _wrkmem = nullptr;
};

#endif
