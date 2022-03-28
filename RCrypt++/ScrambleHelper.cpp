#include <vector>
#include "ScrambleHelper.h"

string ScrambleHelper::ReverseScramble(string str) {
    reverse(str.begin(), str.end());
    vector<char> list;

    for (int i = 0; i < str.length(); i++)
    {
        if (str[i] == '\'' && i + 1 != str.length())
        {
            list.push_back(str[i + 1]);
            ++i;
        }
        else if (str[i] == '2' && i + 1 != str.length() && str[i + 1] != '2')
        {
            list.push_back(str[i + 1]);
            list.push_back('2');
            ++i;
        }
        else
        {
            list.push_back(str[i]);
            list.push_back('\'');
        }
    }
    char* result = new char[list.size()];
    for (int i = 0; i < list.size(); i++) {
        result[i] = list.at(i);
    }
    return string(result, list.size());
}
