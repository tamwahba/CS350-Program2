#ifndef _DATABLOCK_H_
#define _DATABLOCK_H_

#include "Block.h"
#include <string>
#include <vector>

class DataBlock: public Block {
    private:
        std::string blockString;
    public:
        friend std::istream& operator>>(std::istream& input, DataBlock& dataBlock);
        friend std::ostream& operator<<(std::ostream& output, const DataBlock& dataBlock);    
};

#endif
