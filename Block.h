#ifndef _BLOCK_H_
#define _BLOCK_H_

#include <string>
#include <iostream>

class Block {
    public:
        Block();
        ~Block();
        int blockSize = 1024;

        void overwite(char character, unsigned start, unsigned size);

        friend std::istream& operator>>(std::istream& input, Block& block);
        friend std::ostream& operator<<(std::ostream& output, const Block& block);

    protected:
        char* data;
};

#endif
