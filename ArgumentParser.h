#ifndef ARGUMENTPARSER_H
#define ARGUMENTPARSER_H

#include <string>

class ArgumentParser
{

    public:

        ArgumentParser(int argc, char* args[]);

        std::string getNextArgument();
        std::string peekNextArgument();

        bool hasArgumentsLeft();
        void ungetArgument();

    private:

        int mCount, mCurrentArg;
        char** mArgs;

};

#endif // ARGUMENTPARSER_H
