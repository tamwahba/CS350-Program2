#include "LFS.h"

LFS::LFS()
    : checkpointFile("DRIVE/CHECKPOINT_REGION",
        std::ios::binary | std::ios::in | std::ios::out),
    isClean(32, true),
    iMapAddresses(40, 0),
    currentIMapIdx{0},
    currentSegmentIdx{0},
    currentBlockIdx{0},
    numCleanSegments{32} {
        // read or create segments
        for (int i = 0; i < 32; i++) {
            segments.push_back(new Segment("DRIVE/SEGMENT" + std::to_string(i + 1)));
        }
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
                unsigned address = 0;
                checkpointFile.read((char*)&address, sizeof(address));
                iMapAddresses[i] = address;
                if (iMapAddresses[i] != 0) {
                    currentIMapIdx = i;
                }
            }
            checkpointFile.clear();
            checkpointFile.seekg(0, std::ios::beg);
            // read filenames
            for (unsigned i = 0; i <= currentIMapIdx; i++) {
                unsigned iMapAddress = iMapAddresses[i];
                unsigned iMapSegmentIdx = getSegmentIndexFromAddress(iMapAddress);
                unsigned iMapBlockIdx = getBlockIndexFromAddress(iMapAddress);
                IMap iMap(segments[iMapSegmentIdx]->blocks[iMapBlockIdx]);
                for (unsigned j = 0; j < iMap.iNodeAddresses.size(); j++) {
                    unsigned iNodeAddress = iMap.iNodeAddresses[j];
                    if (iNodeAddress != 0) {
                        unsigned iNodeSegmentIdx = getSegmentIndexFromAddress(iNodeAddress);
                        unsigned iNodeBlockIdx = getBlockIndexFromAddress(iNodeAddress);
                        INode iNode(segments[iNodeSegmentIdx]->blocks[iNodeBlockIdx]);
                        files[iNode.fileName] = iNodeAddress;
                    }
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

void LFS::import(std::string& lfsFileName, std::istream& data) {
    unsigned iMapAddress = iMapAddresses[currentIMapIdx];
    unsigned iMapSegmentIdx = getSegmentIndexFromAddress(iMapAddress);
    unsigned iMapBlockIdx = getBlockIndexFromAddress(iMapAddress);
    IMap iMap(segments[iMapSegmentIdx]->blocks[iMapBlockIdx]);
    if (iMapAddress == 0) {
        iMap = IMap();
    }
    if (!iMap.hasFree()) {
        iMap = IMap();
        currentIMapIdx++; // may go out of bounds.
    }
    INode iNode(lfsFileName);

    while (data) {
        Block dataBlock;
        data >> dataBlock;
        unsigned blockOffset = 
            segments[currentSegmentIdx]->addBlock(
                dataBlock, iNode.fileSize, iMap.getNextINodeIndex());
        if (blockOffset == 0) {
            selectNewCleanSegment();
            blockOffset = segments[currentSegmentIdx]->addBlock(
                dataBlock, iNode.fileSize, iMap.getNextINodeIndex());
        }
        unsigned blockAddress = (currentSegmentIdx << 10) + blockOffset;
        iNode.addBlockAddress(blockAddress);
    }

    unsigned iNodeOffset =segments[currentSegmentIdx]->addBlock(
        iNode, iMap.getNextINodeIndex());
    if (iNodeOffset == 0) {
        selectNewCleanSegment();
        iNodeOffset = segments[currentSegmentIdx]->addBlock(
            iNode, iMap.getNextINodeIndex());
    }
    unsigned iNodeAddress = (currentSegmentIdx << 10) + iNodeOffset;
    iMap.addINodeWithAddress(iNodeAddress);

    unsigned iMapOffset = segments[currentSegmentIdx]->addBlock(iMap, currentIMapIdx);
    if (iMapOffset == 0) {
        selectNewCleanSegment();
        iMapOffset = segments[currentSegmentIdx]->addBlock(iMap, currentIMapIdx);
    }
    iMapAddress = (currentSegmentIdx << 10) + iMapOffset;
    iMapAddresses[currentIMapIdx] = iMapAddress;

    files[lfsFileName] = iNodeAddress;
}

std::string LFS::list() {
    std::stringstream fNames;
    for (auto file: files) {
        unsigned iNodeAddress = file.second;
        unsigned int segmentIdx = getSegmentIndexFromAddress(iNodeAddress);
        unsigned int blockIdx = getBlockIndexFromAddress(iNodeAddress);
        INode iNode(segments[segmentIdx]->blocks[blockIdx]);
        fNames << file.first << " ";
        fNames << iNode.fileSize;
        fNames << std::endl;
    }
    return fNames.str();
}

void LFS::remove(std::string& lfsFileName) {
    unsigned iNodeAddress = files[lfsFileName];
    unsigned iMapIndex = getImapIndexFromINodeAddress(iNodeAddress);
    unsigned iMapAddress = iMapAddresses[iMapIndex];
    unsigned iMapSegmentIndex = getSegmentIndexFromAddress(iMapAddress);
    unsigned iMapBlockIdx = getBlockIndexFromAddress(iMapAddress);
    IMap iMap(segments[iMapSegmentIndex]->blocks[iMapBlockIdx]);
    unsigned iNodeIndex = iMap.getIndexForINodeAddress(iNodeAddress);
    iMap.removeINodeAtIndex(iNodeIndex);
    
    unsigned iMapOffset = segments[currentSegmentIdx]->addBlock(iMap, iMapIndex);
    if (iMapOffset == 0) {
        selectNewCleanSegment();
        iMapOffset = segments[currentSegmentIdx]->addBlock(iMap, iMapIndex);
    }
    iMapAddress = (currentSegmentIdx << 10) + iMapOffset;
    iMapAddresses[iMapIndex] = iMapAddress;
    files.erase(lfsFileName);
}

std::string LFS::cat(std::string lfsFileName) {
    std::stringstream data;
    unsigned iNodeAddress = files[lfsFileName];
    unsigned segmentIdx = getSegmentIndexFromAddress(iNodeAddress);
    unsigned blockIdx = getBlockIndexFromAddress(iNodeAddress);
    INode iNode(segments[segmentIdx]->blocks[blockIdx]);
    unsigned i = 0;
    while(iNode.data[i] != '\0') i++; //Scanning past filename
    while(iNode.data[i] == '\0') i++; //Scanning to file size
    unsigned numBlocks = 0;
    i -= 3;
    for(int j = 6; j >= 0; j -= 2) numBlocks += (iNode.data[i++] << j);
    for(unsigned j = 0; j < numBlocks; j++) {
        unsigned dataBlockAdd = 0; 
        for(int k = 6; k >= 0; k -= 2) {
            dataBlockAdd += (iNode.data[i++] << k);
        }
        unsigned dataSegIdx = getSegmentIndexFromAddress(dataBlockAdd);
        unsigned dataBlockIdx = getBlockIndexFromAddress(dataBlockAdd);
        data << segments[dataSegIdx]->blocks[dataBlockIdx];
    }
    return data.str();
}

std::string LFS::display(std::string lfsFileName, int howMany, int start) {
	std::string result = "";
	if (files.find(lfsFileName) != files.end()) {
		unsigned iNodeAddress = files[lfsFileName];
		unsigned iNodeSegmentIdx = getSegmentIndexFromAddress(iNodeAddress);
		unsigned iNodeBlockIdx = getBlockIndexFromAddress(iNodeAddress);
		INode iNode(segments[iNodeSegmentIdx]->blocks[iNodeBlockIdx]);
		for (auto blockAddress: iNode.blockAddresses) {
			if (blockAddress == 0) {
				break;
			}
			unsigned blockSegmentIdx = getSegmentIndexFromAddress(blockAddress);
			unsigned blockIdx = getBlockIndexFromAddress(blockAddress);
			result += 
				segments[blockSegmentIdx]->blocks[blockIdx].getFormattedBytesOfLength(1024) + '\n';
		}
	} else {
		result = "File " + lfsFileName + " does not exitst.";
	}
	return result;
}

void LFS::overwrite(std::string lfsFileName, int howMany, int start, char c) {
    unsigned iNodeAddress = files[lfsFileName];
    unsigned iNodeSegmentIdx = getSegmentIndexFromAddress(iNodeAddress);
    unsigned iNodeBlockIdx = getBlockIndexFromAddress(iNodeAddress);
    INode iNode(segments[iNodeSegmentIdx]->blocks[iNodeBlockIdx]);

    unsigned iMapIndex = getImapIndexFromINodeAddress(iNodeAddress);
    unsigned iMapAddress = iMapAddresses[iMapIndex];
    unsigned iMapSegmentIndex = getSegmentIndexFromAddress(iMapAddress);
    unsigned iMapBlockIndex = getBlockIndexFromAddress(iMapAddress);
    IMap iMap(segments[iMapSegmentIndex]->blocks[iMapBlockIndex]);

    unsigned iNodeIndex = 0;
    for(int i = 0; i < iMap.iNodeAddresses.size(); i++) {
        if(iMap.iNodeAddresses[i] == iNodeAddress)
            iNodeIndex = i;
    }

    unsigned i = 0;
    while(iNode.data[i] != '\0') i++; //Scanning past filename
    while(iNode.data[i] == '\0') i++; //Scanning to file size
    unsigned numBlocks = 0;
    i -= 3;
    for(int j = 6; j >= 0; j -= 2) numBlocks += (iNode.data[i++] << j);

    int numBlocksRem = numBlocks;
    int howManyRem = howMany;
    int remStart = start;
    int workingBlockIndex = 0;


    //Don't need to create new data block or update INode
    while(remStart > 1023 && numBlocksRem > 0) {
        std::cout << "Scanning past block not to be changed at " << workingBlockIndex << std::endl;
        remStart -= 1024;
        numBlocksRem--;
        workingBlockIndex++;
    }

    //Create new data blocks until start point is within next block
    while(remStart > 1023) {
        std::cout << "Creating new empty block until start" << std::endl;
        Block dataBlock;
        unsigned blockOffset = segments[currentSegmentIdx]->addBlock(
                dataBlock, iNode.fileSize, iMap.getNextINodeIndex());
        if (blockOffset == 0) {
            selectNewCleanSegment();
            blockOffset = segments[currentSegmentIdx]->addBlock(
                dataBlock, iNode.fileSize, iMap.getNextINodeIndex());
        }
        unsigned blockAddress = (currentSegmentIdx << 10) + blockOffset;
        iNode.updateBlockAddressAtIndex(blockAddress, workingBlockIndex);
        
        remStart -= 1024;
        workingBlockIndex++;
    }

    while(howManyRem > 0) {
        std::cout << "Creating changed block at " << workingBlockIndex << std::endl;
        unsigned workingBitIndex = 0;
        Block dataBlock;
        while(remStart > 0) {
            dataBlock.data[workingBitIndex++] = '\0';
            remStart--;
        }
        while(workingBitIndex <= 1024 && howManyRem >= 0) {
            dataBlock.data[workingBitIndex++] = c;
            howManyRem--;
        }
        while(workingBitIndex <= 1024) {
            dataBlock.data[workingBitIndex++] = '\0';
        }
        unsigned blockOffset = segments[currentSegmentIdx]->addBlock(
                dataBlock, iNode.fileSize, iMap.getNextINodeIndex());
        if (blockOffset == 0) {
            selectNewCleanSegment();
            blockOffset = segments[currentSegmentIdx]->addBlock(
            dataBlock, iNode.fileSize, iMap.getNextINodeIndex());
        }
        unsigned blockAddress = (currentSegmentIdx << 10) + blockOffset;
        iNode.updateBlockAddressAtIndex(blockAddress, workingBlockIndex);
        numBlocksRem--;
        workingBlockIndex++;
    }

    /*while(numBlocksRem >= 0) {
        
    }*/
    
    std::cout << "Writing INode" << std::endl;
    unsigned iNodeOffset = segments[currentSegmentIdx]->addBlock(
        iNode, iMap.getNextINodeIndex());
    if (iNodeOffset == 0) {
        selectNewCleanSegment();
        iNodeOffset = segments[currentSegmentIdx]->addBlock(
            iNode, iMap.getNextINodeIndex());
    }
    iNodeAddress = (currentSegmentIdx << 10) + iNodeOffset;
    iMap.updateINodeAddressAtIndex(iNodeAddress, iNodeIndex);

    std::cout << "Writing IMap" << std::endl;
    unsigned iMapOffset = segments[currentSegmentIdx]->addBlock(iMap, currentIMapIdx);
    if (iMapOffset == 0) {
        selectNewCleanSegment();
        iMapOffset = segments[currentSegmentIdx]->addBlock(iMap, currentIMapIdx);
    }
    iMapAddress = (currentSegmentIdx << 10) + iMapOffset;
    iMapAddresses[currentIMapIdx] = iMapAddress;

    files[lfsFileName] = iNodeAddress;
}

void LFS::flush() {
    // write segments
    for (auto segment: segments) {
        segment->write();
    }
    // write checkpoint region 
    flushCheckpoint();
}

/*void LFS::clean() {


  }*/

unsigned LFS::getBlockIndexFromAddress(unsigned address) {
    return address & 0x3FF;
}

unsigned LFS::getSegmentIndexFromAddress(unsigned address) {
    return address >> 10;
}

unsigned LFS::getImapIndexFromINodeAddress(unsigned address) {
    for(unsigned i = 0; i < iMapAddresses.size(); i++) {
        unsigned iMapAddress = iMapAddresses[i];
        unsigned iMapSegmentIdx = getSegmentIndexFromAddress(iMapAddress);
        unsigned iMapBlockIdx = getBlockIndexFromAddress(iMapAddress);
        IMap iMap(segments[iMapSegmentIdx]->blocks[iMapBlockIdx]);
        for(auto iNodeAddress : iMap.iNodeAddresses) {
            if(iNodeAddress == address) return i;
        }
    }
    return 0;
}

void LFS::selectNewCleanSegment() {

}

void LFS::updateClean() {

}

void LFS::flushCheckpoint() {
    for (unsigned i = 0; i < isClean.size(); i++) {
        bool clean = isClean[i];
        checkpointFile.write((char*)&clean, sizeof(clean));
    }
    for (unsigned i = 0; i < iMapAddresses.size(); i++) {
        unsigned address = iMapAddresses[i];
        checkpointFile.write((char*)&address, sizeof(address));
    }
}
