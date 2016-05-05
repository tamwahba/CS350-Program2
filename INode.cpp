#include "INode.h"

INode::INode(std::string name) 
    : Block(),
    fileName{name},
    fileSize{0},
    fileSizeIdx{0} {
    	// add filename including null character
        const char* n = name.c_str();
        for (unsigned i = 0; i <= sizeof(n) && i < blockSize - maxFileBlocks; i++) {
            data[i] = n[i];
            currentIdx = i;
        }
        fileSizeIdx = currentIdx; // location of file size (right after file name)
        currentIdx += sizeof(fileSize); // move data write head to next free byte 
        writeFileSize();
}

INode::INode(Block& b)
    : Block(b),
    fileName{data},
    fileSize{0},
    fileSizeIdx{0} {
    	readFileSize();
    	currentIdx = fileSizeIdx + fileSize;
}

unsigned INode::addBlockWithAddress(unsigned address) {
    if (currentIdx < maxFileBlocks) {
        data[currentIdx] = address;
        currentIdx++;
        fileSize++;
        writeFileSize();
    }
    return currentIdx - 1;
}

void INode::updateBlockAddressAtIndex(unsigned address, unsigned index) {
    data[index] = address;
}

void INode::writeFileSize() {
	char* sizePtr = (char *)&fileSize;
	for (unsigned i = 0; i < sizeof(fileSize); i++) {
		data[fileSizeIdx + i] = sizePtr[i];
	}
}

void INode::readFileSize() {
	fileSizeIdx = fileName.size() + 1;
	char* sizePtr = (char*)&fileSize;
	for (unsigned i = 0; i < sizeof(fileSize); i++) {
		sizePtr[i] = data[fileSizeIdx + i];
	}
	fileSize = *((unsigned*)sizePtr);
}
