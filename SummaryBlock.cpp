#include "SummaryBlock.h"

std::istream& operator>>(std::istream& input, SummaryBlock& SummaryBlock) {
    for (int i = 0; i < SummaryBlock.blockSize; ++i) {
        unsigned int val;
        input.read((char*)&val, sizeof(unsigned int));
        SummaryBlock.blockIndices.push_back(val);
    }
    return input;
}

std::ostream& operator<<(std::ostream& output, const SummaryBlock& SummaryBlock) {
    for (auto iter = SummaryBlock.blockIndices.begin();
            iter != SummaryBlock.blockIndices.end(); ++iter) {
        unsigned int val = *iter;
        output.write((char *)&val, sizeof(unsigned int));
    }
    output.write(NULL, SummaryBlock.blockSize - SummaryBlock.blockIndices.size());
    return output;
}


