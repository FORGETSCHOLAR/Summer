#pragma once

#include <fcntl.h>

class FileUtils{
public:
    static bool setNonBlock(int fd)
    {
        int f1 = fcntl(fd,F_GETFL);
        if(f1 < 0) return false;
        fcntl(fd,F_SETFL,f1 | O_NONBLOCK);
        return true;
    }
};