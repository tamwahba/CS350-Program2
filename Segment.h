#ifndef _SEGMENT_H_
#define _SEGMENT_H_

#include "SummaryBlock.h"
#include "Block.h"
#include "INode.h"
#include <iostream>
#include <vector>

class Segment {
public:
    SummaryBlock SSB;
    std::vector<Block> blocks; //max 1023
    unsigned int maxBlocks = 1020;

    
    friend std::istream& operator>>(std::istream& input, Segment& segment);
    friend std::ostream& operator<<(std::ostream& output, const Segment& segment);
    bool addBlock(Block& block, unsigned int howMany);
    void clean();    
};

#endif
