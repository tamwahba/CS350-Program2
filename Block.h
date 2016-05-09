#ifndef _BLOCK_H_
#define _BLOCK_H_

#include <string>
#include <iostream>

class Block {
    public:
        Block();
        Block(Block& b);
        ~Block();

        void overwrite(char character, unsigned start, unsigned size);
        std::string getStringOfLength(unsigned lengthBytes);

        Block& operator=(const Block& other);
        friend std::istream& operator>>(std::istream& input, Block& block);
        friend std::ostream& operator<<(std::ostream& output, const Block& block);

        const unsigned blockSize = 1024;

    public:
        char* data;
};

#endif
