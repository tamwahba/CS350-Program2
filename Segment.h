#ifndef _SEGMENT_H_
#define _SEGMENT_H_

#include <fstream>
#include <vector>

#include "SummaryBlock.h"
#include "Block.h"
#include "INode.h"

class Segment {
    public:
        Segment(std::string fileName);
        
        bool addBlock(Block& block, unsigned int howMany);
        // unsigned deadBlockCount(LFS& fileSystem);
        void write();
        
    private:
        unsigned maxBlocks;
        unsigned summaryBlockCount;
        unsigned currentBlockIdx;
        std::string segmentFileName;
        std::fstream file;

    public:
        std::vector<Block> blocks; //size 1024; 0 - 1023
};

#endif
