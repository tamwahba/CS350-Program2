#include "Block.h"

std::istream& operator>>(std::istream& input, Block& block) {
    char* buffer = new char[block.blockSize]();
    input.read(buffer, block.blockSize);
    block.blockString = std::string(buffer, block.blockSize);
    delete buffer;
    //if(block.blockString.length() != 0) std::cout << block.blockString;
    return input;
}

std::ostream& operator<<(std::ostream& output, const Block& block) {
    output.write(block.blockString.c_str(), block.blockString.length());
    for(int j = 0; j < block.blockSize - block.blockString.length(); j++) {
        output.put('\0');
    }
    return output;
}

