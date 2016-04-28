#ifndef _INODE_H_
#define _INODE_H_

class INode {
private:
    string fileName;
    int fileSize; //in blocks
    Block *blockPointers[128];

public:
    INode();
    ~INode();

};

#endif
