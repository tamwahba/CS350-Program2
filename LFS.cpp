#include "LFS.h"

LFS::LFS()
    : checkpointFile{"DRIVE/CHECKPOINT_REGION",
        std::ios::binary | std::ios::in | std::ios::out},
    isClean(32, true),
    iMapAddresses(40, 0),
    currentIMapIdx{0},
    currentSegmentIdx{0},
    currentBlockIdx{0},
    numCleanSegments{32} {
        // read or create checkpoint region
        if (!checkpointFile) {
            checkpointFile.close();
            checkpointFile.open("DRIVE/CHECKPOINT_REGION",
                std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc); 
        } else {
            for (unsigned i = 0; i < isClean.size() && checkpointFile; i++) {
                bool clean = isClean[i];
                checkpointFile.read((char*)&clean, sizeof(clean));
                isClean[i] = clean;
                if (!isClean[i]) {
                    numCleanSegments--;
                }
            }
            for (unsigned i = 0; i < iMapAddresses.size() && checkpointFile; i++) {
                checkpointFile.read((char*)&iMapAddresses[i], sizeof(iMapAddresses[i]));
                if (iMapAddresses[i] != 0) {
                    currentIMapIdx = i;
                }
            }
            checkpointFile.clear();
            checkpointFile.seekg(0, std::ios::beg);
        }
        // read or create segments
        for (int i = 0; i < 32; i++) {
            segments.push_back(new Segment("DRIVE/SEGMENT" + std::to_string(i + 1)));
        }
        // read filenames
        for (unsigned i = 0; i < currentIMapIdx; i++) {
            unsigned iMapAddress = iMapAddresses[currentIMapIdx];
            unsigned iMapSegmentIdx = getSegmentIndexFromAddress(iMapAddress);
            unsigned iMapBlockIdx = getBlockIndexFromAddress(iMapAddress);
            IMap iMap(segments[iMapSegmentIdx]->blocks[iMapBlockIdx]);
            for (unsigned i = 0; i < iMap.iNodeAddresses.size(); i++) {
                unsigned iNodeAddress = iMap.iNodeAddresses[i];
                if (iNodeAddress != 0) {
                    unsigned iNodeSegmentIdx = getSegmentIndexFromAddress(iNodeAddress);
                    unsigned iNodeBlockIdx = getBlockIndexFromAddress(iNodeAddress);
                    INode iNode(segments[iNodeSegmentIdx]->blocks[iNodeBlockIdx]);
                    files[iNode.fileName] = iNodeAddress;
                }
            }
        }
}

LFS::~LFS() {
    checkpointFile.close();
    for (auto segment: segments) {
        delete segment;
    }
}

void LFS::updateClean() {
}

void LFS::import(std::string& lfsFileName, std::istream& data) {
    unsigned iMapAddress = iMapAddresses[currentIMapIdx];
    unsigned iMapSegmentIdx = getSegmentIndexFromAddress(iMapAddress);
    unsigned iMapBlockIdx = getBlockIndexFromAddress(iMapAddress);
    IMap iMap(segments[iMapSegmentIdx]->blocks[iMapBlockIdx]);
    if (!iMap.hasFree()) {
        iMap = IMap();
        currentIMapIdx++; // may go out of bounds.
    }
    INode iNode(lfsFileName);

    while (data) {
        Block dataBlock;
        data >> dataBlock;
        unsigned blockOffset = segments[currentSegmentIdx]->addBlock(dataBlock, 0);
        // if (blockOffset == 0) {
        //     selectNewCleanSegment();
        //     blockOffset = segments[currentSegmentIdx]->addBlock(dataBlock, 0);
        // }
        unsigned blockAddress = (currentSegmentIdx << 10) + blockOffset;
        iNode.addBlockAddress(blockAddress);
    }

    unsigned iNodeOffset = segments[currentSegmentIdx]->addBlock(iNode, 0);
    if (iNodeOffset == 0) {
        selectNewCleanSegment();
        iNodeOffset = segments[currentSegmentIdx]->addBlock(iNode, 0);
    }
    unsigned iNodeAddress = (currentSegmentIdx << 10) + iNodeOffset;
    iMap.addINodeWithAddress(iNodeAddress);

    unsigned iMapOffset = segments[currentSegmentIdx]->addBlock(iMap, 0);
    if (iMapOffset == 0) {
        selectNewCleanSegment();
        iMapOffset = segments[currentSegmentIdx]->addBlock(iMap, 0);
    }
    iMapAddress = (currentSegmentIdx << 10) + iMapOffset;
    iMapAddresses[currentIMapIdx] = iMapAddress;

    files[lfsFileName] = iNodeAddress;
}

std::string LFS::list() {
    std::stringstream fNames;
    for (auto file: files) {
        unsigned int segmentIdx = file.second >> 10;
        unsigned int blockIdx = file.second & 0x3FF;
        INode iNode(segments[segmentIdx]->blocks[blockIdx]);
        fNames << file.first << " ";
        fNames << iNode.fileSize;
        fNames << std::endl;
    }
    return fNames.str();
}

void LFS::remove(std::string& lfsFileName) {
    unsigned iNodeAddress = files[lfsFileName];
    unsigned iMapAddress = getImapAddressFromINodeAddress(iNodeAddress);
    unsigned segmentIndex = getSegmentIndexFromAddress(iMapAddress);
    unsigned blockIndex = getBlockIndexFromAddress(iMapAddress);
    IMap iMap(segments[segmentIndex]->blocks[blockIndex]);
    iMap.removeINodeAtIndex(getBlockIndexFromAddress(iNodeAddress));
    if (!segments[currentSegmentIdx]->addBlock(iMap, 0)) {
        selectNewCleanSegment();
        segments[currentSegmentIdx]->addBlock(iMap, 0);
    }
    files.erase(lfsFileName);
}

std::string LFS::cat(std::string lfsFileName) {
    std::stringstream data;
    unsigned iNodeAddress = files[lfsFileName];
    unsigned segmentIndex = getSegmentIndexFromAddress(iNodeAddress);
    unsigned blockIndex = getBlockIndexFromAddress(iNodeAddress);
    INode iNode(segments[segmentIndex]->blocks[blockIndex]);
    unsigned i = 0;
    while(iNode.data[i] != '\0') i++; //Scanning past filename
    while(iNode.data[i] == '\0') i++; //Scanning to file size
    unsigned numBlocks = 0;
    i-=3;
    for(int j = 6; j >= 0; j-=2) {
        numBlocks += (iNode.data[i++] << j);
    }
    for(unsigned j = 0; j < numBlocks; j++) {
        unsigned dataBlockIndex = 0; 
        for(int k = 6; k >= 0; k-=2) {
            dataBlockIndex += (iNode.data[i++] << k);
        }
        data << segments[segmentIndex]->blocks[dataBlockIndex];
    }
    return data.str();
}

/*std::string LFS::display(std::string lfsFileName, int howMany, int start) {


  }*/

/*void LFS::overwrite(std::string lfsFileName, int howMany, int start, char c) {


  }*/

void LFS::flush() {
    // write segments
    for (auto segment: segments) {
        segment->write();
    }
    // write checkpoint region 
    for (unsigned i = 0; i < isClean.size(); i++) {
        bool clean = isClean[i];
        checkpointFile.write((char*)&clean, sizeof(clean));
    }
    for (unsigned i = 0; i < iMapAddresses.size(); i++) {
        checkpointFile.write((char*)&iMapAddresses[i], sizeof(iMapAddresses[i]));
    }
}

/*void LFS::clean() {


  }*/

unsigned LFS::getBlockIndexFromAddress(unsigned address) {
    return address & 0x3FF;
}

unsigned LFS::getSegmentIndexFromAddress(unsigned address) {
    return address >> 10;
}

unsigned LFS::getImapAddressFromINodeAddress(unsigned index) {
    return 0;
}

void LFS::selectNewCleanSegment() {

}

