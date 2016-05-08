#ifndef _SEGMENT_H_
#define _SEGMENT_H_

#include "LFS.h"
#include "SummaryBlock.h"
#include "Block.h"
#include "INode.h"
#include <fstream>
#include <vector>

class Segment {
    public:
        Segment(std::string fileName, unsigned blockSize, unsigned segmentSize);
        
        bool addBlock(Block& block, unsigned int howMany);
        // unsigned deadBlockCount(LFS& fileSystem);
        void write();
        

    private:
        std::vector<Block> blocks; //size 1024; 0 - 1023
    	std::string segmentFileName;
        std::fstream file;
        unsigned maxBlocks;
        unsigned summaryBlockCount;
        unsigned currentBlockIdx;
};

#endif
