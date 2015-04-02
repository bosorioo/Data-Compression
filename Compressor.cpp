#include "Compressor.h"
#include <algorithm>
#include <iostream>
#include "BitStream.h"

Compressor::Compressor()
{
    int endianTest = 1;
    mIsLittleEndian = *(char*)(&endianTest) == 1;
}

int Compressor::reverseEndianess(int value)
{
    int newValue;
    char* in_ptr = (char*)&value;
    char* out_ptr = (char*)&newValue;

    out_ptr[0] = in_ptr[3];
    out_ptr[1] = in_ptr[2];
    out_ptr[2] = in_ptr[1];
    out_ptr[3] = in_ptr[0];

    return newValue;
}

std::vector<unsigned char> Compressor::compress(void* data, unsigned length)
{
    mFrequency = getFrequency(data, length);
    int bits = getBestRatio(length);
    char* ptr = (char*)data;
    int i;

    if (bits == -1)
        return std::vector<unsigned char>();

    BitStream bs;

    unsigned totalSize = 27 + ((1 << bits) - 1) * 8;

    if (mIsLittleEndian)
        bs.insert((int)length, 24);
    else
        bs.insert(reverseEndianess((int)length), 24);

    bs.insert(bits, 3);

    for (i = 0; i < (1 << bits) - 1; i++)
    {
        if ((int)mFrequency.size() > i)
            bs.insert(mFrequency[i].character);
        else
            bs.insert(0);
    }

    std::vector<char> uncompressed;

    for (i = 0; i < (int)length; i++)
    {
        int vecPos = findChar(mFrequency, *ptr, (1 << bits) - 1);
        if (vecPos == -1)
        {
            bs.insert(0U, bits);
            uncompressed.push_back(*ptr);
        }
        else
        {
            bs.insert(vecPos + 1, bits);
        }

        totalSize += bits;
        ptr++;
    }

    for (i = 0; i < (int)uncompressed.size(); i++)
    {
        bs.insert(uncompressed[i]);
        totalSize += 8;
    }

    return bs.getData();
}

std::vector<unsigned char> Compressor::decompress(void* data, unsigned length)
{
    BitStream bs;
    bs.insert(data, length);
    bs.read_bytes(&length, 3);

    if (!mIsLittleEndian)
        length = (unsigned)reverseEndianess((int)length);

    unsigned char bits;
    bs.read_bits(bits, 3);

    if (bits < 1 || bits > 7)
    {
        char bStr[4];
        throw (std::string("Bit-size of enconding isn't valid: ") + std::string(itoa(bits, bStr, 10)));
    }

    unsigned char byte;
    int bitsRead = 27;
    int i;
    std::vector<char> charTable;
    std::vector<char> compressedChars;

    for (i = 1; i <= (1 << bits) - 1; i++)
    {
        bs.read_bits(byte, 8);
        charTable.push_back(byte);
        bitsRead += 8;

        if (!bs.canRead())
            throw std::string ("Bad file for decompression [1]");
    }

    for (unsigned j = 1; j <= length; j++)
    {
        bs.read_bits(byte, bits);
        bitsRead += bits;
        compressedChars.push_back(byte);

        if (!bs.canRead())
            throw std::string ("Bad file for decompression [2]");
    }

    std::vector<unsigned char> result(length, 0);

    for (i = 0; i < (int)compressedChars.size(); i++)
    {
        if (compressedChars[i] == 0 || (unsigned char)compressedChars[i] > charTable.size())
        {
            if (!bs.canRead())
                throw std::string ("Bad file for decompression [3]");

            bs.read_bits(result[i], 8);
            bitsRead += 8;
        }
        else
        {
            result[i] = charTable[compressedChars[i] - 1];
        }
    }

    return result;
}

Compressor::FrequencyVector Compressor::getFrequency(void* data, unsigned length)
{
    char* ptr = (char*)data;
    FrequencyVector freq;

    while (length-- > 0)
    {
        getChar(freq, *ptr).count++;
        ptr++;
    }

    std::sort(freq.begin(), freq.end(), Compressor::compareFreq);
    return freq;
}

bool Compressor::compareFreq(FrequencyChar a, FrequencyChar b)
{
    return a.count >= b.count;
}

Compressor::FrequencyChar& Compressor::getChar(FrequencyVector& vec, char character)
{
    for (unsigned i = 0; i < vec.size(); i++)
    {
        if (vec[i].character == character)
            return vec[i];
    }

    vec.push_back(FrequencyChar(character));
    return vec.back();
}

int Compressor::findChar(FrequencyVector& vec, char character, unsigned limit)
{
    for (unsigned i = 0; i < vec.size() && i < limit; i++)
    {
        if (vec[i].character == character)
            return i;
    }

    return -1;
}

int Compressor::getBestRatio(unsigned dataLength)
{
    int bestBit = 0;
    int compressedLength = -1;

    for (int bits = 1; bits <= 7; bits++)
    {
        int compressed = 0;
        int neededBits;

        for (int i = 0; i < (1 << bits) - 1; i++)
        {
            if ((int)mFrequency.size() <= i)
                break;

            compressed += mFrequency[i].count;
        }

        neededBits = computeSize(bits, dataLength, compressed);
        if (neededBits < compressedLength || compressedLength == -1)
        {
            compressedLength = neededBits;
            bestBit = bits;
        }
    }

    return bestBit;
}

unsigned Compressor::computeSize(int numberOfBits, unsigned dataLength, unsigned compressCount)
{
    return numberOfBits * dataLength + (dataLength - compressCount) * 8 + 27 + ((1 << numberOfBits) - 1) * 8;
}
