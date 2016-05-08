#ifndef _IMAP_H_
#define _IMAP_H

#include <vector>
#include <iostream>

#include "Block.h"
#include "INode.h"

class IMap: public Block {
    public:
        IMap();
        IMap(Block& b);

        unsigned addINodeWithAddress(unsigned address);
        void updateINodeAddressAtIndex(unsigned address, unsigned index);
        void removeINodeAtIndex(unsigned index);

    private:
        std::vector<INode*> iNodes;
        unsigned currentIdx;
        unsigned freeCount;    
};

#endif
