#include "IMap.h"

IMap::IMap()
    : Block(),
    currentIdx{0},
    freeCount{blockSize} {
    
}

IMap::IMap(Block& b)
    : Block(),
    currentIdx{0},
    freeCount{blockSize} {
        for (unsigned i = 0; i < blockSize; i++) {
            data[i] = b.data[i];
            if (data[i] != '\0') {
                freeCount--;
                currentIdx = i;
            }
        }
    }

unsigned IMap::addINodeWithAddress(unsigned address) {
    if (currentIdx < blockSize) {
        data[currentIdx] = address;
        currentIdx++;
        freeCount--;
    }
    return currentIdx - 1;
}

void IMap::updateINodeAddressAtIndex(unsigned address, unsigned index) {
    data[index] = address;
}


void IMap::removeINodeAtIndex(unsigned index) {
    data[index] = '\0';
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