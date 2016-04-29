#ifndef _INODE_H_
#define _INODE_H_

#include "Block.h"
#include <string>

class INode {
private:
    std::string fileName;
    int fileSize; //in blocks
    Block *blockPointers[128];

public:
    INode();
    ~INode();

};

#endif
