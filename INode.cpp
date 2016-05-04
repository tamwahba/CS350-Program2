#include "INode.h"

INode::INode() {
    blockIndices.resize(128);
    fileSize = 0;
}

std::istream& operator>>(std::istream& input, INode& iNode) {
    for (int i = 0; i < iNode.blockSize; ++i) {
        unsigned int val;
        input.read((char*)&val, sizeof(unsigned int));
        iNode.blockIndices[i] = val;
    }
    char* buffer = new char[iNode.blockSize - iNode.fileSize]();
    input.read(buffer, iNode.blockSize - iNode.fileSize);
    iNode.fileName = std::string(buffer, iNode.blockSize - iNode.fileSize);
    return input;
}

std::ostream& operator<<(std::ostream& output, const INode& iNode) {
    for (auto iter = iNode.blockIndices.begin();
            iter != iNode.blockIndices.end(); ++iter) {
        unsigned int val = *iter;
        output.write((char *)&val, sizeof(unsigned int));
    }
    output.write(iNode.fileName.c_str(), iNode.fileName.length());
    for(int i = 0; i < iNode.blockSize - iNode.blockIndices.size() - iNode.fileName.size(); i++) {
        output.put('\0');
    }
    
    return output;
}

