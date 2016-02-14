#ifndef KITCMP_H
#define KITCMP_H

#include <string.h>

struct	kitcmp
{
public:
        bool operator()(const char* s1, const char* s2) const
        {
                return strcmp(s1, s2) < 0;
        }
};

#endif // KITCMP_H
