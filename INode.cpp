#include "INode.h"

INode::INode(std::string name) 
    : Block(),
    fileName{name},
    fileSize{0},
    fileSizeIdx{0} {
    	// add file name including null character
        const char* n = name.c_str();
        for (unsigned i = 0; i <= sizeof(n) && i < blockSize - maxFileBlocks; i++) {
            data[i] = n[i];
        }
        currentIdx = sizeof(n) + 1;
        data[currentIdx++] = '\0'; // add null terminator
        fileSizeIdx = currentIdx; // location of file size (right after file name)
        currentIdx += sizeof(fileSize); // move data write head to next free byte 
        writeFileSize();
}

INode::INode(Block& b)
    : Block(b),
    fileName(data),
    fileSize{0},
    fileSizeIdx{0} {
        readFileSize();
        currentIdx = fileSizeIdx + sizeof(fileSizeIdx) + (fileSize * sizeof(unsigned));
}

unsigned INode::addBlockAddress(unsigned address) {
    unsigned addressIdx = currentIdx;
    if (currentIdx < maxFileBlocks) {
        for (unsigned i = 0; i < sizeof(address); i++) {
            data[currentIdx + i] = ((char*)&address)[i];
        }
        currentIdx += sizeof(address);
        fileSize++;
        writeFileSize();
    }
    return addressIdx;
}

void INode::updateBlockAddressAtIndex(unsigned address, unsigned index) {
    for (unsigned i = 0; i < sizeof(address); i++) {
        data[index + i] = ((char*)&address)[i];
    }
}

void INode::writeFileSize() {
	for (unsigned i = 0; i < sizeof(fileSize); i++) {
		data[fileSizeIdx + i] = ((char*)&fileSize)[i];
	}
}

void INode::readFileSize() {
	fileSizeIdx = fileName.size() + 1;
	for (unsigned i = 0; i < sizeof(fileSize); i++) {
		((char*)&fileSize)[i] = data[fileSizeIdx + i];
	}
}
