#include "IMap.h"

IMap::IMap()
    : Block(),
    currentIdx{0},
    freeCount{blockSize} {
    
}

IMap::IMap(Block& b)
    : Block(b),
    iNodeAddresses{blockSize/4},
    currentIdx{0},
    freeCount{blockSize} {
        for (unsigned i = 0; i < blockSize; i += sizeof(unsigned)) {
            unsigned address = 0;
            for (unsigned j = 0; j < sizeof(unsigned); j++) {
                ((char*)&address)[j] = data[i + j];
            }
            if (address != 0) {
                iNodeAddresses[i] = address;
                freeCount--;
                currentIdx = i + sizeof(unsigned);
            }
        }
    }

unsigned IMap::addINodeWithAddress(unsigned address) {
    unsigned addressIdx = currentIdx;
    // TODO - add to free spots when at end of list
    if (currentIdx < blockSize) {
        for (unsigned i = 0; i < sizeof(address); i++) {
            data[currentIdx + i] = ((char*)&address)[i];
        }
        currentIdx += sizeof(address);
        freeCount--;
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

bool IMap::hasFree() {
    // return freeCount > 0;
    return currentIdx < blockSize;
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