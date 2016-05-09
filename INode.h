#ifndef _INODE_H_
#define _INODE_H_

#include <iostream>
#include <string>
#include <vector>

#include "Block.h"

class INode : public Block {
    public:
        INode(std::string name);
        INode(Block& b);

        unsigned addBlockAddress(unsigned address);
        void updateBlockAddressAtIndex(unsigned address, unsigned index);

        std::string fileName;

    private:
        unsigned fileSize; //in blocks
        unsigned fileSizeIdx;
        unsigned currentIdx;
        unsigned maxFileBlocks = 128;

        void writeFileSize();
        void readFileSize();
};

#endif
