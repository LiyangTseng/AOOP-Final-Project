#ifndef EASYCITY2_H
#define EASYCITY2_H
#include "mydatabase.h"
#include <string>

class EasyCity2 : public MyDatabase
{
public:
    EasyCity2();
    string solve(string s);
};

#endif // EASYCITY2_H