#include "Block.h"

std::istream& operator>>(std::istream& input, Block& block) {
    char* buffer = new char[block.blockSize]();
    input.read(buffer, block.blockSize);
    block.blockString = std::string(buffer);
    delete[] buffer;
    if(block.blockString.length() != 0) std::cout << block.blockString << std::endl;
    return input;
}

std::ostream& operator<<(std::ostream& output, const Block& block) {
    output.write(block.blockString.c_str(), block.blockString.length());
    for(unsigned i = 0; i < block.blockSize - block.blockString.length(); i++) {
        output.put('\0');
    }
    return output;
}

