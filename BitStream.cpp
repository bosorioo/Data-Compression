#include "BitStream.h"
#include <iostream>
#include <vector>
#include <cmath>

#define BITS_IN_BYTE 8

BitStream::BitStream()
{
    mBitsLeft = BITS_IN_BYTE;
    mCurrentByte = mStreamPosition = 0;
}

void BitStream::insert(unsigned value, char bits)
{
    value <<= (sizeof(value) * BITS_IN_BYTE) - bits;
    value >>= (sizeof(value) * BITS_IN_BYTE) - bits;

    while (bits > 0)
    {
        if (mBitsLeft > bits)
        {
            mCurrentByte |= (value << (mBitsLeft - bits));
            mBitsLeft -= bits;
            return;
        }
        else
        {
            mCurrentByte |= ((unsigned)value >> (bits - mBitsLeft));
            mBytes.push_back(mCurrentByte);
            mCurrentByte = 0;
            bits -= mBitsLeft;
            mBitsLeft = BITS_IN_BYTE;
        }
    }
}

void BitStream::insert(unsigned char byte)
{
    char bitsLeft = mBitsLeft;

    if (bitsLeft < BITS_IN_BYTE)
    {
        insert(byte >> (BITS_IN_BYTE - bitsLeft), bitsLeft);
        insert(byte, BITS_IN_BYTE - bitsLeft);
    }
    else
    {
        mBytes.push_back(byte);
    }
}

void BitStream::insert(void* data, unsigned length)
{
    unsigned char* ptr = (unsigned char*)data;

    while (length > 0)
    {
        insert(*ptr++);
        length--;
    }
}

void BitStream::read_bytes(void* bytes_out, unsigned length)
{
    unsigned char* ptr = (unsigned char*)bytes_out;

    while (length > 0)
    {
        length--;
        read_bits(ptr[length], BITS_IN_BYTE);
    }
}

void BitStream::read_bits(unsigned char& bits_out, char bits)
{
    unsigned byteIndex = (mStreamPosition / BITS_IN_BYTE);
    int bitIndex = mStreamPosition % BITS_IN_BYTE;

    if (byteIndex <= mBytes.size())
    {
        if (byteIndex == mBytes.size())
            bits_out = mCurrentByte;
        else
            bits_out = mBytes[byteIndex];

        bits_out <<= bitIndex;
        bits_out >>= bitIndex;

        if (bits + bitIndex > BITS_IN_BYTE)
            bits_out = bits_out << (bits + bitIndex - BITS_IN_BYTE);
        else
            bits_out = bits_out >> (BITS_IN_BYTE - bits - bitIndex);

        bits -= (BITS_IN_BYTE - bitIndex);
        mStreamPosition += bits + (BITS_IN_BYTE - bitIndex);

        if (mStreamPosition > (mBytes.size() + 1) * BITS_IN_BYTE)
            mStreamPosition = (mBytes.size() + 1) * BITS_IN_BYTE;

        if (bits > 0)
        {
            if (byteIndex >= mBytes.size())
                return;
            else if (byteIndex + 1 == mBytes.size())
                bits_out |= (mCurrentByte >> (BITS_IN_BYTE - bits));
            else
                bits_out |= (mBytes[byteIndex + 1] >> (BITS_IN_BYTE - bits));

        }

    }
}

std::string BitStream::getBinaryString()
{
    char byte, bit;
    int index;
    std::string binaryString(mBytes.size() * BITS_IN_BYTE + (mBitsLeft < BITS_IN_BYTE ? BITS_IN_BYTE : 0), 0);

    for (index = 0; index < (int)mBytes.size(); index++)
    {
        byte = mBytes[index];

        for (bit = 0; bit < BITS_IN_BYTE; bit++)
        {
            binaryString[index * BITS_IN_BYTE + bit] = (byte & 0x80) ? '1' : '0';
            byte <<= 1;
        }
    }

    byte = mCurrentByte;
    if (mBitsLeft != BITS_IN_BYTE) for (bit = 0; bit < BITS_IN_BYTE; bit++)
    {
        if (bit >= BITS_IN_BYTE - mBitsLeft)
            binaryString[index * BITS_IN_BYTE + bit] = 'x';
        else
            binaryString[index * BITS_IN_BYTE + bit] = (byte & 0x80) ? '1' : '0';

        byte <<= 1;
    }

    return binaryString;
}

std::string BitStream::getHexString()
{
    char byte;
    int index;
    std::string hexString(mBytes.size() * 2 + (BITS_IN_BYTE - mBitsLeft + 3) / 4, 0);

    for (index = 0; index < (int)mBytes.size(); index++)
    {
        byte = mBytes[index];
        hexString[index * 2] = ((byte & 0xF0) >> 4) + '0';
        hexString[index * 2 + 1] = (byte & 0x0F) + '0';

        if (hexString[index * 2] > '9')
            hexString[index * 2] += 'A' - '0' - 10;

        if (hexString[index * 2 + 1] > '9')
            hexString[index * 2 + 1] += 'A' - '0' - 10;
    }

    if (mBitsLeft < BITS_IN_BYTE)
    {
        hexString[index * 2] = ((mCurrentByte & 0xF0) >> 4) + '0';

        if (hexString[index * 2] > '9')
            hexString[index * 2] += 'A' - '0' - 10;

        if (hexString.size() == mBytes.size() * 2 + 2)
        {
            hexString[index * 2 + 1] = (mCurrentByte & 0x0F) + '0';

            if (hexString[index * 2 + 1] > '9')
                hexString[index * 2 + 1] += 'A' - '0' - 10;
        }
    }

    return hexString;
}

std::vector<unsigned char> BitStream::getData()
{
    if (mBitsLeft == BITS_IN_BYTE)
        return mBytes;

    std::vector<unsigned char> data = mBytes;
    data.push_back(mCurrentByte);
    return data;
}

int BitStream::getBitCount()
{
    return mBytes.size() * BITS_IN_BYTE + (BITS_IN_BYTE - mBitsLeft);
}

bool BitStream::canRead()
{
    unsigned byteIndex = (mStreamPosition / BITS_IN_BYTE);
    int bitIndex = mStreamPosition % BITS_IN_BYTE;

    return byteIndex < mBytes.size() || (byteIndex == mBytes.size() && bitIndex < (BITS_IN_BYTE - mBitsLeft));
}
