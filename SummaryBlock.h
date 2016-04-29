#ifndef _SUMMARYBLOCK_H_
#define _SUMMARYBLOCK_H_

#include "Block.h"
#include <iostream>
#include <vector>

class SummaryBlock: public Block {
    private:
        std::vector<unsigned int> blockIndices;

    public:
        friend std::istream& operator>>(std::istream& input, SummaryBlock& summaryBlock);
        friend std::ostream& operator<<(std::ostream& output, const SummaryBlock& summaryBlock);
        
};

#endif
