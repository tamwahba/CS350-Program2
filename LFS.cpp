#include "LFS.h"

LFS::LFS()
    : checkpointFile{"DRIVE/CHECKPOINT_REGION",
        std::ios::binary | std::ios::in | std::ios::out},
    currentSegmentIdx{0},
    currentBlockIdx{0},
    numCleanSegments{32} {
        // TODO PROCESS CHECKPOINT REGION
        for (int i = 0; i < 32; i++) {
            segments.push_back(new Segment("DRIVE/SEGMENT" + std::to_string(i + 1)));
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
}

std::string LFS::list() {
    std::stringstream fNames;
    for (auto file: files) {
        fNames << file.first << std::endl;
    }
    return fNames.str();;
}

void LFS::remove(std::string& lfsFileName) {
    unsigned fileAddress = files[lfsFileName];
    unsigned iMapAddress = getImapAddressFromINodeAddress(fileAddress);
    unsigned segmentIndex = getSegmentIndexFromAddress(iMapAddress);
    unsigned blockIndex = getBlockIndexFromAddress(iMapAddress);
    IMap iMap(segments[segmentIndex]->blocks[blockIndex]);
    iMap.removeINodeAtIndex(getBlockIndexFromAddress(fileAddress));
    if (!segments[currentSegmentIdx]->addBlock(iMap, 0)) {
        selectNewCleanSegment();
    }
    files.erase(lfsFileName);
}

/*std::string LFS::cat(std::string lfsFileName) {


  }*/

/*std::string LFS::display(std::string lfsFileName, int howMany, int start) {


  }*/

/*void LFS::overwrite(std::string lfsFileName, int howMany, int start, char c) {


  }*/

void LFS::flush() {
    for (auto segment: segments) {
        segment->write();
    }
}

/*void LFS::clean() {


  }*/

unsigned LFS::getBlockIndexFromAddress(unsigned address) {
    return 0;
}

unsigned LFS::getSegmentIndexFromAddress(unsigned address) {
    return 0;
}

unsigned LFS::getImapAddressFromINodeAddress(unsigned index) {
    return 0;
}

void LFS::selectNewCleanSegment() {

}

