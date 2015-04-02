#ifndef _BITSTREAM_INCLUDE
#define _BITSTREAM_INCLUDE

#include <string>
#include <vector>

class BitStream
{
public:

    BitStream();

    void insert(unsigned value, char bits);
    void insert(unsigned char byte);
    void insert(void* data, unsigned length);

    void read_bytes(void* bytes_out, unsigned length);
    void read_bits(unsigned char& bits_out, char bits);

    std::string getBinaryString();
    std::string getHexString();
    std::vector<unsigned char> getData();

    int getBitCount();

    bool canRead();

private:

    std::vector<unsigned char> mBytes;
    char mBitsLeft;
    char mCurrentByte;
    unsigned mStreamPosition;

};

#endif // _BITSTREAM_INCLUDE
