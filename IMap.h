#ifndef _IMAP_H_
#define _IMAP_H_

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
        bool hasFree();

        std::vector<unsigned> iNodeAddresses;

    private:
        unsigned currentIdx;
        unsigned freeCount;    
};

#endif
