#pragma once
#include <iostream>


class Error
{
public:

    char* msg;


    Error(const char* errmsg)
    {
        msg = new char[200];

        strcpy(msg, errmsg);
    }

    Error() = default;

    void what();
};

inline void Error::what()
{
    std::cout << "Error message: [" << msg << "]" << std::endl;;
    abort();
}
