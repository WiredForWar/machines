#include <string>
#include <iostream>
// #include <function.hpp>

#include "ctl/set.hpp"

void testSet();
void testMultiSet();

int main()
{
    testSet();
    testMultiSet();

    return 0;
}

void testSet()
{
    ctl_set<int, std::less<int>> intSet;

    intSet.insert(4);
    intSet.insert(1);
    intSet.insert(-50);
    intSet.insert(3063764);
    intSet.insert(2);
    intSet.insert(4);
    intSet.insert(4);
    intSet.insert(4);

    ctl_set<int, std::less<int>>::iterator i;

    for (i = intSet.begin(); i != intSet.end(); ++i)
        std::cout << (*i) << std::endl;

    std::cout << std::endl;
}

void testMultiSet()
{
    ctl_multiset<int, std::less<int>> intSet;

    intSet.insert(4);
    intSet.insert(1);
    intSet.insert(-50);
    intSet.insert(3063764);
    intSet.insert(2);
    intSet.insert(4);
    intSet.insert(4);
    intSet.insert(4);

    ctl_multiset<int, std::less<int>>::iterator i;

    for (i = intSet.begin(); i != intSet.end(); ++i)
        std::cout << (*i) << std::endl;
}
