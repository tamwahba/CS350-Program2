#ifndef _IMAP_H_
#define _IMAP_H_

#include <vector>
#include <iostream>

#include "Block.h"

class IMap: public Block {
    public:
        std::vector<unsigned int> iNodes;
        friend std::istream& operator>>(std::istream& input, IMap& iMap);
        friend std::ostream& operator<<(std::ostream& output, const IMap& iMap);            
};

#endif
