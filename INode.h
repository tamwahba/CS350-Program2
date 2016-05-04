#ifndef _INODE_H_
#define _INODE_H_

#include "Block.h"
#include <iostream>
#include <string>
#include <vector>

class INode : public Block {
public:
    INode();
    std::string fileName;
    int fileSize; //in blocks
    std::vector<unsigned int> blockIndices;
    friend std::istream& operator>>(std::istream& input, INode& iNode);
    friend std::ostream& operator<<(std::ostream& output, const INode& iNode);
};

#endif
