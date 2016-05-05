#include "IMap.h"

std::istream& operator>>(std::istream& input, IMap& iMap) {
    for (int i = 0; i < iMap.blockSize; ++i) {
        unsigned int val = 0;
        input.read((char *)&val, sizeof(unsigned int));
        iMap.iNodes[i] = val;
    }
    return input;
}

std::ostream& operator<<(std::ostream& output, const IMap& iMap) {
    for (auto iter = iMap.iNodes.begin(); iter != iMap.iNodes.end(); ++iter) {
        unsigned int val = *iter;
        output.write((char *)(&val), sizeof(unsigned int));
    }
    return output;
}


