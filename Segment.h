#ifndef _SEGMENT_H_
#define _SEGMENT_H_

#include "SummaryBlock.h"
#include "Block.h"
#include "INode.h"
#include <iostream>
#include <vector>

class Segment {
private:
    SummaryBlock SSB;
    std::vector<Block> blocks; //max 1023
    int counter;

public:
    friend std::istream& operator>>(std::istream& input, Segment& segment);
    friend std::ostream& operator<<(std::ostream& output, const Segment& segment);
    void clean();    
};

#endif
