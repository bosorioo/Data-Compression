#include "ArgumentParser.h"

ArgumentParser::ArgumentParser(int argc, char* args[]) :
    mCount(argc), mArgs(args)
{
    mCurrentArg = 0;
}

std::string ArgumentParser::getNextArgument()
{
    if (mCurrentArg + 1 >= mCount)
        return std::string();

    mCurrentArg++;

    return mArgs[mCurrentArg];
}

std::string ArgumentParser::peekNextArgument()
{
    if (mCurrentArg + 1 >= mCount)
        return std::string();

    return mArgs[mCurrentArg + 1];
}

bool ArgumentParser::hasArgumentsLeft()
{
    return mCurrentArg + 1 < mCount;
}

void ArgumentParser::ungetArgument()
{
    if (mCurrentArg > 0)
        mCurrentArg--;
}

