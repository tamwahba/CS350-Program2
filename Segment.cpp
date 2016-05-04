#include "Segment.h"

std::istream& operator>>(std::istream& input, Segment& segment) {
    input >> segment.SSB;
    std::cout << "Constructing each block" << std::endl;
    for (unsigned int i = 0; i < segment.maxBlocks; i++) {
        Block block;
        input >> block;
        if(!block.blockString.empty()) {
            segment.blocks.push_back(Block());
            std::cout << block.blockString.size() << ", ";
        }
    }
    return input;
}


std::ostream& operator<<(std::ostream& output, const Segment& segment) {
    output << segment.SSB;
    for (auto iter = segment.blocks.begin(); iter != segment.blocks.end(); ++iter) {
        output << *iter;
    }
    return output;
}

bool Segment::addBlock(Block& block, unsigned int howMany) {
    if((blocks.size() + howMany) < maxBlocks) {
        blocks.push_back(block);
        return true;
    } else return false;
}
