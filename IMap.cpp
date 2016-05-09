#include "IMap.h"

IMap::IMap()
    : Block(),
    currentIdx{0},
    freeCount{blockSize} {
    
}

IMap::IMap(Block& b)
    : Block(b),
    currentIdx{0},
    freeCount{blockSize} {
        for (unsigned i = 0; i < blockSize; i += sizeof(unsigned)) {
            unsigned index = 0;
            for (unsigned j = 0; j < sizeof(unsigned); j++) {
                ((char*)&index)[j] = data[i + j];
            }
            if (index != 0) {
                freeCount--;
                currentIdx = i + sizeof(unsigned);
            }
        }
    }

unsigned IMap::addINodeWithAddress(unsigned address) {
    unsigned addressIdx = currentIdx;
    if (currentIdx < blockSize) {
        for (unsigned i = 0; i < sizeof(address); i++) {
            data[currentIdx + i] = ((char*)&address)[i];
        }
        currentIdx += sizeof(address);
        freeCount--;
        writeFileSize();
    }
    return addressIdx;
}

void IMap::updateINodeAddressAtIndex(unsigned address, unsigned index) {
    for (unsigned i = 0; i < sizeof(address); i++) {
        data[index + i] = ((char*)&address)[i];
    }
}


void IMap::removeINodeAtIndex(unsigned index) {
    for (unsigned i = 0; i < sizeof(unsigned); i++) {
        data[index + i] = '\0';
    }
    freeCount++;
}

// bool hasFreeIndecies() {
//     return freeCount > 0;
// }

// unsigned findFreeIndex() {
//     if (currentIdx < blockSize) {
//         return currentIdx;
//     }

//     for (unsigned i = 0; i < blockSize; i++) {
//         if (data[i] == '\0') {
//             return i;
//         }
//     }
// }