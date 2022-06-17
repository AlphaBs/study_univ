#include "string"
#include "iostream"

int find(std::string s, std::string pat)
{
    int patternIndex = 0;
    for (int i = 0; i < s.length(); i++)
    {
        if (s.at(i) == pat.at(patternIndex))
        {
            patternIndex++;
            if (patternIndex == pat.length())
                return i - pat.length() + 1;
        }
        else
        {
            patternIndex = 0;
            if (i > s.length() - pat.length())
                break;
        }
    }

    return -1;
}

int main()
{
    std::string a, b;
    std::getline(std::cin, a);
    std::getline(std::cin, b);
    std::cout << find(a, b) << std::endl;
}