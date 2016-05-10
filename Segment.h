#ifndef _SEGMENT_H_
#define _SEGMENT_H_

#include <fstream>
#include <vector>
#include <limits>

#include "SummaryBlock.h"
#include "Block.h"
#include "INode.h"
#include "IMap.h"

class Segment {
    public:
        Segment(std::string fileName);
        ~Segment();
        
        unsigned addBlock(Block& block, unsigned blockIndex, unsigned iNodeIndex);
        unsigned addBlock(INode& block, unsigned index);
        unsigned addBlock(IMap& block, unsigned index);
        
        // unsigned deadBlockCount(LFS& fileSystem);
        void write();
        
    private:
        unsigned maxBlocks;
        unsigned summaryBlockCount;
        unsigned currentBlockIdx;
        std::string segmentFileName;
        std::fstream file;

        unsigned addBlock(Block& block, unsigned blockIndex,
            unsigned iNodeIndex, unsigned howMany);

    public:
        std::vector<Block> blocks; //size 1024; 0 - 1023
};

#endif
