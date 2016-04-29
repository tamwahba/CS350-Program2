#ifndef _SEGMENT_H_
#define _SEGMENT_H_

#include "SummaryBlock.h"
#include "Block.h"
#include "INode.h"

class Segment {
private:
    SummaryBlock SSB;
    Block blocks[1022];
    INode iNode;
    int counter;

public:
    Segment();
    ~Segment();

};

#endif
