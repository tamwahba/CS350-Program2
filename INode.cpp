#include "INode.h"

INode::INode() 
    : Block() {

}

INode::INode(std::string name) 
    : Block(),
    fileName{name},
    fileSize{0} {
        char[] n = name.c_str();
        for (int i = 0; i < name.length() && i < blockSize - maxFileBlocks; i++) {
            data[i] = char[i];
            currentIdx = i;
        }
}

INode(&Block b)
    : Block(),
    fileName{b.data}
    fileSize{0} {
        for (int i = 0; i < fileName.length(); i++) {
            data[i] = b.data[i];
        }

        for (unsigned i = fileName.length(); i < blockSize; i++) {
            data[i] = b.data[i];
            if (data[i] != '\0') {
                fileSize++;
                currentIdx = i;
            }
        }
}

unsigned INode::addBlockWithAddress(unsigned address) {
    if (currentIdx < blockSize) {
        data[currentIdx] = address;
        currentIdx++;
        freeCount--;
    }
    return currentIdx - 1;
}

void INode::updateBlockAddressAtIndex(unsigned address, unsigned index) {
    data[index] = address;
}
