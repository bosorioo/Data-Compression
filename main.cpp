#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include "Compressor.h"
#include "ArgumentParser.h"

std::string getConsoleInput();
void printHelp();
bool parseArguments(int argc, char* args[], bool& compress, std::string& input, std::string& output);

void compressFile(std::string input, std::string output);
void decompressFile(std::string input, std::string output);

int main(int argc, char* args[])
{
    if (argc == 1)
    {
        printHelp();
        return 0;
    }

    bool compress;
    std::string input, output;

    if (!parseArguments(argc, args, compress, input, output))
    {
        std::cout << "Invalid arguments.\n";
        return 0;
    }

    if (compress)
        compressFile(input, output);
    else
        decompressFile(input, output);

    return 0;
}

std::string getConsoleInput()
{
    std::string input;
    char c;

    do {
        std::cin.get(c);
        if (c == '\n')
            break;
        input += c;
    } while (1);

    return input;
}

void printHelp()
{
    std::cout << "Simple Compression Algorithm parameters:\n\n";
    std::cout << "-c input [output]\tCompresses file <input>, output is stored on file <output>, if provided, or in <input>.bca\n\n";
    std::cout << "-d input [output]\tDecompresses file in <input>, output is stored on file <output>, if provided, or asked in runtime\n\n";
    std::cout << "-c overwrites -d and vice-versa, only last parameters are considered\n";
}

bool parseArguments(int argc, char* args[], bool& compress, std::string& input, std::string& output)
{
    ArgumentParser parser(argc, args);
    std::string arg;
    bool hasCommand = false;

    while (parser.hasArgumentsLeft())
    {
        arg = parser.getNextArgument();

        if (!arg.empty() && *arg.begin() == '-') // commands
        {
            if (arg == "-d")
            {
                compress = false;
                hasCommand = true;
                input = parser.getNextArgument();

                if (input.empty() || *input.begin() == '-')
                {
                    std::cout << "Invalid input file for -d (decompression command).\n";
                    return false;
                }

                arg = parser.peekNextArgument();

                if (!arg.empty() && *input.begin() != '-')
                    output = parser.getNextArgument();
                else
                    output.clear();
            }
            else if (arg == "-c")
            {
                compress = true;
                hasCommand = true;
                input = parser.getNextArgument();

                if (input.empty() || *input.begin() == '-')
                {
                    std::cout << "Invalid input file for -c (compression command).\n";
                    return false;
                }

                arg = parser.peekNextArgument();

                if (!arg.empty() && *input.begin() != '-')
                    output = parser.getNextArgument();
                else
                    output = input + ".bca";
            }
        }
    }

    return hasCommand;
}

void compressFile(std::string input, std::string output)
{
    Compressor compressor;

    std::fstream inputFile(input.c_str(), std::fstream::in | std::fstream::binary);
    std::fstream outputFile(output.c_str(), std::fstream::in);
    std::vector<unsigned char> fileData;
    char userAnswer;

    if (!inputFile.is_open())
    {
        std::cout << "Input file from path \"" << input << "\" couldn't be opened.\n";
        return;
    }

    while (outputFile.is_open())
    {
        std::cout << "Output path \"" << output << "\" already exists. Do you want to overwrite it? (y/n)\n\t> ";
        userAnswer = *getConsoleInput().begin();

        if (userAnswer == 'y' || userAnswer == 'Y')
        {
            outputFile.close();
        }
        else
        {
            std::cout << "Enter new output path:\n\t> ";
            output = getConsoleInput();
            outputFile.close();
            outputFile.open(output.c_str(), std::fstream::in);
        }
    }

    outputFile.open(output.c_str(), std::fstream::out | std::fstream::binary);

    if (!outputFile.is_open())
    {
        std::cout << "Unable to open output file \"" << output << "\".\n";
        return;
    }

    int byte;

    while ((byte = inputFile.get()) != EOF)
        fileData.push_back((unsigned char)byte);

    inputFile.close();

    unsigned originalFileSize = fileData.size();

    fileData = compressor.compress(fileData.data(), fileData.size());

    outputFile.write((char*)fileData.data(), fileData.size());
    outputFile.close();

    std::cout << "File \"" << output << "\" saved successfully.\n";

    if (originalFileSize > 0)
        std::cout << "New file is " << ((float)fileData.size() / originalFileSize) << "% of original file size.\n";
}

void decompressFile(std::string input, std::string output)
{
    Compressor compressor;

    if (output.empty())
    {
        std::cout << "Enter output path:\n\t> ";
        output = getConsoleInput();

        while (output.empty())
        {
            std::cout << "Invalid output path, enter it again:\n\t> ";
            output = getConsoleInput();
        }
    }

    std::fstream inputFile(input.c_str(), std::fstream::in | std::fstream::binary);
    std::fstream outputFile(output.c_str(), std::fstream::in);
    std::vector<unsigned char> fileData;
    char userAnswer;

    if (!inputFile.is_open())
    {
        std::cout << "Input file from path \"" << input << "\" couldn't be opened.\n";
        return;
    }

    while (outputFile.is_open())
    {
        std::cout << "Output path \"" << output << "\" already exists. Do you want to overwrite it? (y/n)\n\t> ";
        userAnswer = *getConsoleInput().begin();

        if (userAnswer == 'y' || userAnswer == 'Y')
        {
            outputFile.close();
        }
        else
        {
            std::cout << "Enter new output path:\n\t> ";
            output = getConsoleInput();
            outputFile.close();
            outputFile.open(output.c_str(), std::fstream::in);
        }
    }

    outputFile.open(output.c_str(), std::fstream::out | std::fstream::binary);

    if (!outputFile.is_open())
    {
        std::cout << "Unable to open output file \"" << output << "\".\n";
        return;
    }

    int byte;

    while ((byte = inputFile.get()) != EOF)
        fileData.push_back((unsigned char)byte);

    inputFile.close();

    unsigned originalFileSize = fileData.size();
    bool success = false;

    try {
        fileData = compressor.decompress(fileData.data(), fileData.size());
        success = true;
    } catch (std::string& excep) {
        std::cout << "Couldn't decompress file.\n";
        std::cout << "More details: " << excep << "\n";
    }

    if (!success)
        return;

    outputFile.write((char*)fileData.data(), fileData.size());
    outputFile.close();

    std::cout << "File \"" << output << "\" saved successfully.\n";

    if (originalFileSize > 0)
        std::cout << "New file is " << ((float)fileData.size() / originalFileSize) << "% of original file size.\n";
}
