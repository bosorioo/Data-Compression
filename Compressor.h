#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include <vector>

class Compressor
{
public:

    Compressor();

    int reverseEndianess(int value);

    std::vector<unsigned char> compress(void* data, unsigned length);
    std::vector<unsigned char> decompress(void* data, unsigned length);

private:

    struct FrequencyChar
    {
        FrequencyChar() : character(0), count(0) {}
        FrequencyChar(char c) : character(c), count(0) {}
        char character;
        unsigned count;
    };
    typedef std::vector<FrequencyChar> FrequencyVector;

    FrequencyVector mFrequency;

    static bool compareFreq(FrequencyChar a, FrequencyChar b);

    bool mIsLittleEndian;

    FrequencyChar& getChar(FrequencyVector& vec, char character);
    int findChar(FrequencyVector& vec, char character, unsigned limit);
    FrequencyVector getFrequency(void* data, unsigned length);
    unsigned computeSize(int numberOfBits, unsigned dataLength, unsigned compressCount);
    int getBestRatio(unsigned length);
};

#endif // COMPRESSOR_H
