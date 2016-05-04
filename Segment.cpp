#include "Segment.h"

Segment::Segment(std::string fileName, unsigned blockSize, unsigned segmentSize)
    : file{fileName, std::ios::binary | std::ios::in | std::ios::out},
    maxBlocks{blockSize/segmentSize},
    summaryBlockCount{8 * maxBlocks},
    currentBlockIdx{summaryBlockCount} {
    std::cout << "Segment constructor " << fileName << std::endl;
    std::cout << "maxBlocks" << maxBlocks << std::endl;
    std::cout << "summaryBlockCount" << summaryBlockCount << std::endl;
    std::cout << "currentBlockIdx" << currentBlockIdx << std::endl;
    if (!file) {
        std::cout << "Segment constructor " << fileName << "doesn't exist" << std::endl;
        file.close();
        file.open(fileName, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc); 
    } else {
        while (file) {
            Block block;
            file >> blocks[currentBlockIdx];
            currentBlockIdx++;
        }
    }
}

void Segment::wrtie() {
    for (auto block: blocks) {
        file << block;
    }
}

bool Segment::addBlock(Block& block, unsigned int howMany) {
    std::cout << "Segment constructor " << fileName;
    std::cout << " adding block at index" << currentBlockIdx << std::endl;
    if (currentBlockIdx + howMany < maxBlocks) {
        blocks[currentBlockIdx] = block;
        std::cout << "Segment constructor " << fileName;
        std::cout << "added" << std::endl;
        return true;
    }
    std::cout << "Segment constructor " << fileName;
    std::cout << "no space" << std::endl;
    return false;
}
