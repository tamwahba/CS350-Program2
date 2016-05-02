#include "Segment.h"

std::istream& operator>>(std::istream& input, Segment& segment) {
    input >> segment.SSB;
    for (int i = 0; i < 1023; i++) {
        segment.blocks[i] = Block();
        input >> segment.blocks[i];
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

