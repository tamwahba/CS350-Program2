#include "LFS.h"

LFS::LFS()
    : checkpointFile{"DRIVE/CHECKPOINT_REGION",
        std::ios::binary | std::ios::in | std::ios::out},
    isClean(32, true),
    iMapAddresses(40, 0),
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
            }
            checkpointFile.clear();
            checkpointFile.seekg(0, std::ios::beg);
        }
        // read or create segments
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

