#include "mario.h"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
using namespace std;
Mario::Mario()
{

}

string Mario::solve(string s)
{
    stringstream ss;
    ss<<s;
    int N;
    ss>>N;
    string result = "";
    vector<bool> coinType(1001,false);
    int coin;
    while (ss>>coin)
    {
    }
    return result;
}
