#ifndef _INODE_H_
#define _INODE_H_

#include <iostream>
#include <string>
#include <vector>

#include "Block.h"

/*
 *
 *  /-- file name, followed by a 0
 * |-|-|--------------------------------- .... ----|
 *    ^ number of blocks in file           \---a list of block indices
 */
class INode : public Block {
    public:
        INode(std::string name);
        INode(Block& b);

        unsigned addBlockAddress(unsigned address);
        void updateBlockAddressAtIndex(unsigned address, unsigned index);

        std::string fileName;
        unsigned fileSize; //in blocks


        unsigned getBlockListStartIdx();
    private:
        unsigned fileSizeIdx;
        unsigned blockListStartIdx;
        unsigned currentIdx;
        unsigned maxFileBlocks = 128;

        void writeFileSize();
        void readFileSize();
};

#endif
