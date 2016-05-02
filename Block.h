#ifndef _BLOCK_H_
#define _BLOCK_H_

#include <string>
#include <iostream>

class Block {
    protected:
        int blockSize = 1024;
        std::string blockString;
    public:
        friend std::istream& operator>>(std::istream& input, Block& block);
        friend std::ostream& operator<<(std::ostream& output, const Block& block);
};

#endif
