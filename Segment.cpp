#include "Segment.h"

Segment::Segment(std::string fileName)
    : maxBlocks{1024},
    summaryBlockCount{8},
    currentBlockIdx{0},
    segmentFileName{fileName},
    file{segmentFileName, std::ios::binary | std::ios::in | std::ios::out},
    blocks{maxBlocks} {
    // std::cout << "Segment constructor " << segmentFileName << std::endl;
    // std::cout << "numBlocks: " << blocks.size() << std::endl;
    // std::cout << "maxBlocks: " << maxBlocks << std::endl;
    // std::cout << "summaryBlockCount: " << summaryBlockCount << std::endl;
    // std::cout << "currentBlockIdx: " << currentBlockIdx << std::endl;
    if (!file) {
        // std::cout << segmentFileName << "doesn't exist" << std::endl;
        file.close();
        // std::cout << "making " << segmentFileName << std::endl;
        file.open(segmentFileName, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc); 
    } else {
        while (file && currentBlockIdx < maxBlocks) {
            file >> blocks[currentBlockIdx];
            currentBlockIdx++;
        }
    }
    file.clear();
    file.seekg(0, std::ios::beg);
    // std::cout << "End constructor " << segmentFileName << std::endl;
}

void Segment::write() {
    for (auto block: blocks) {
        file << block;
    }
}

bool Segment::addBlock(Block& block, unsigned int howMany) {
    std::cout << "Segment " << segmentFileName;
    std::cout << " adding block at index" << currentBlockIdx << std::endl;
    if (currentBlockIdx + howMany < maxBlocks) {
        blocks[currentBlockIdx] = block;
        std::cout << "Segment " << segmentFileName << "added" << std::endl;
        return true;
    }
    std::cout << "Segment " << segmentFileName << " no space" << std::endl;
    return false;
}
