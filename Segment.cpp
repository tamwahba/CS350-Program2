#include "Segment.h"

Segment::Segment(std::string fileName)
        : maxBlocks{1024},
        summaryBlockCount{8},
        currentBlockIdx{0},
        segmentFileName{fileName},
        file{segmentFileName, std::ios::binary | std::ios::in | std::ios::out},
        blocks{maxBlocks} {
    Block block;
    block.overwrite(std::numeric_limits<unsigned char>::max(), 0, 1024);
    for (unsigned i = 0; i < summaryBlockCount; i++) {
        blocks[i] = block;
    }
    if (!file) {
        file.close();
        file.open(segmentFileName,
            std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc); 
    } else {
        while (file && currentBlockIdx < maxBlocks) {
            file >> blocks[currentBlockIdx++];
        }
    }
    currentBlockIdx = 8;
    file.clear();
    file.seekg(0, std::ios::beg);
}

Segment::~Segment() {
    file.close();
}

void Segment::write() {
    for (auto block: blocks) {
        file << block;
    }
    file.clear();
    file.seekg(0, std::ios::beg);
}

unsigned Segment::addBlock(Block& block, unsigned blockIndex,
        unsigned iNodeIndex, unsigned howMany) {
    // std::cout << "Segment " << segmentFileName;
    // std::cout << " adding block at index " << currentBlockIdx << std::endl;
    if (currentBlockIdx + howMany < maxBlocks) {
        blocks[currentBlockIdx] = block;
        // std::cout << "Segment " << segmentFileName << " added" << std::endl;
        
        unsigned currentSummaryBlockIdx = currentBlockIdx % summaryBlockCount;
        return currentBlockIdx++;
    }
    // std::cout << "Segment " << segmentFileName << " no space" << std::endl;
    return 0;
}

unsigned Segment::addBlock(Block& block, unsigned blockIndex, unsigned iNodeIndex) {
    return Segment::addBlock(block, blockIndex, iNodeIndex, 0);
}

unsigned Segment::addBlock(INode& block, unsigned index) {
    return Segment::addBlock(block, 128, index, 0);
}

unsigned Segment::addBlock(IMap& block, unsigned index) {
    return Segment::addBlock(block, index, 10 * 1024, 0);
}
