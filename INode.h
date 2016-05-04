#ifndef _INODE_H_
#define _INODE_H_

#include "Block.h"
#include <iostream>
#include <string>
#include <vector>

class INode : public Block {
    private:
        std::string fileName;
        unsigned fileSize; //in blocks
        unsigned currentIdx;
        unsigned maxFileBlocks = 128;

    public:
        INode();
        INode(std::string name);
        INode(Block& b);

        unsigned addBlockWithAddress(unsigned address);
        void updateBlockAddressAtIndex(unsigned address, unsigned index);
};

#endif
