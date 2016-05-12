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
            // find a clean segment
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
        if (iNode.fileSize == 128) {
            data.setstate(std::ios::badbit);
            break;
        }
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
        unsigned lastBlockAdd = iNode.blockAddresses.back();
        unsigned lastSegIdx = getSegmentIndexFromAddress(lastBlockAdd);
        unsigned lastBlockIdx = getBlockIndexFromAddress(lastBlockAdd);
        unsigned fileSize = ((iNode.fileSize - 1) * 1024) + 
            segments[lastSegIdx]->blocks[lastBlockIdx].
            getFormattedBytesOfLength(1024).size();
        fNames << fileSize;
        fNames << std::endl;
    }
    return fNames.str();
}

void LFS::remove(std::string& lfsFileName) {
    unsigned iNodeAddress = findFile(lfsFileName);
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
    iMapAddresses[currentIMapIdx] = iMapAddress;
    files.erase(lfsFileName);
}

std::string LFS::cat(std::string lfsFileName) {
    std::stringstream data;
    unsigned iNodeAddress = findFile(lfsFileName);
    unsigned segmentIdx = getSegmentIndexFromAddress(iNodeAddress);
    unsigned blockIdx = getBlockIndexFromAddress(iNodeAddress);
    INode iNode(segments[segmentIdx]->blocks[blockIdx]);
    unsigned i = 0;
    while(iNode.data[i++] != '\0') ;
    unsigned numBlocks = iNode.data[i++];
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
    std::stringstream result;
    if (files.find(lfsFileName) != files.end()) {
        unsigned iNodeAddress = findFile(lfsFileName);
        unsigned iNodeSegmentIdx = getSegmentIndexFromAddress(iNodeAddress);
        unsigned iNodeBlockIdx = getBlockIndexFromAddress(iNodeAddress);
        INode iNode(segments[iNodeSegmentIdx]->blocks[iNodeBlockIdx]);
        for (auto blockAddress: iNode.blockAddresses) {
            if (blockAddress == 0 || howMany < 1) {
                break;
            }

            unsigned blockSegmentIdx = getSegmentIndexFromAddress(blockAddress);
            unsigned blockIdx = getBlockIndexFromAddress(blockAddress);
            unsigned numToPrint = 1024;
            if(howMany < 1024) numToPrint = howMany;
            howMany -= numToPrint;
            result << segments[blockSegmentIdx]->blocks[blockIdx].getFormattedBytesOfLength(numToPrint);
        }
    } else {
        result << "File " << lfsFileName << " does not exitst.";
    }
    return result.str();
}

void LFS::overwrite(std::string lfsFileName, int howMany, int start, char c) {
    unsigned iNodeAddress = findFile(lfsFileName);
    unsigned iNodeSegmentIdx = getSegmentIndexFromAddress(iNodeAddress);
    unsigned iNodeBlockIdx = getBlockIndexFromAddress(iNodeAddress);
    INode iNode(segments[iNodeSegmentIdx]->blocks[iNodeBlockIdx]);

    unsigned iMapIndex = getImapIndexFromINodeAddress(iNodeAddress);
    unsigned iMapAddress = iMapAddresses[iMapIndex];
    unsigned iMapSegmentIndex = getSegmentIndexFromAddress(iMapAddress);
    unsigned iMapBlockIndex = getBlockIndexFromAddress(iMapAddress);
    IMap iMap(segments[iMapSegmentIndex]->blocks[iMapBlockIndex]);

    unsigned iNodeIndex = 0;
    for(unsigned i = 0; i < iMap.iNodeAddresses.size(); i++) {
        if(iMap.iNodeAddresses[i] == iNodeAddress)
            iNodeIndex = i;
    }

    unsigned i = 0;
    while(iNode.data[i++] != '\0') ; //Scanning past filename
    unsigned numBlocks = iNode.data[i++];

    int numBlocksRem = numBlocks;
    int howManyRem = howMany;
    int remStart = start;
    int workingBlockIndex = 0;

    //Don't need to create new data block or update INode
    while(remStart > 1023 && numBlocksRem > 0) {
        remStart -= 1024;
        numBlocksRem--;
        workingBlockIndex++;
    }

    //Create new data blocks until start point is within next block
    while(remStart > 1023) {
        Block dataBlock;
        unsigned blockOffset = segments[currentSegmentIdx]->addBlock(
                dataBlock, iNode.fileSize, iMap.getNextINodeIndex());
        if (blockOffset == 0) {
            selectNewCleanSegment();
            blockOffset = segments[currentSegmentIdx]->addBlock(
                    dataBlock, iNode.fileSize, iMap.getNextINodeIndex());
        }
        unsigned blockAddress = (currentSegmentIdx << 10) + blockOffset;
        if((unsigned)workingBlockIndex <= numBlocks) 
            iNode.addBlockAddress(blockAddress);
        else
            iNode.updateBlockAddressAtIndex(blockAddress, workingBlockIndex); 
        remStart -= 1024;
        workingBlockIndex++;
    }

    while(howManyRem > 0) {
        unsigned workingBitIndex = 0;
        Block newDataBlock;

        unsigned oldDataBlockAdd = 0;
        for(int j = 6; j >= 0; j -= 2) {
            oldDataBlockAdd += (iNode.data[i++] << j);
        }
        unsigned oldDataSegIdx = 
            getSegmentIndexFromAddress(oldDataBlockAdd);
        unsigned oldDataBlockIdx =
            getBlockIndexFromAddress(oldDataBlockAdd);

        while(remStart > 0) {
            if((unsigned)workingBlockIndex <= numBlocks)
                newDataBlock.data[workingBitIndex] = 
                    segments[oldDataSegIdx]->blocks[oldDataBlockIdx].
                    data[workingBitIndex];
            else 
                newDataBlock.data[workingBitIndex] = '\0';
            workingBitIndex++;
            remStart--;
        }
        while(workingBitIndex <= 1024 && howManyRem >= 0) {
            newDataBlock.data[workingBitIndex++] = c;
            howManyRem--;
        }
        while(workingBitIndex <= 1024) {
            if((unsigned)workingBlockIndex <= numBlocks)
                newDataBlock.data[workingBitIndex] = 
                    segments[oldDataSegIdx]->blocks[oldDataBlockIdx].
                    data[workingBitIndex];
            else 
                newDataBlock.data[workingBitIndex] = '\0';
            workingBitIndex++;
            remStart--;    
        }
        unsigned blockOffset = segments[currentSegmentIdx]->addBlock(
                newDataBlock, iNode.fileSize, iMap.getNextINodeIndex());
        if (blockOffset == 0) {
            selectNewCleanSegment();
            blockOffset = segments[currentSegmentIdx]->addBlock(
                    newDataBlock, iNode.fileSize, iMap.getNextINodeIndex());
        }
        unsigned blockAddress = (currentSegmentIdx << 10) + blockOffset;
        if((unsigned)workingBlockIndex < numBlocks) {
            iNode.updateBlockAddressAtIndex(blockAddress, workingBlockIndex);
        } else iNode.addBlockAddress(blockAddress);        
        numBlocksRem--;
        workingBlockIndex++;
    }

    unsigned iNodeOffset = segments[currentSegmentIdx]->addBlock(
            iNode, iMap.getNextINodeIndex());
    if (iNodeOffset == 0) {
        selectNewCleanSegment();
        iNodeOffset = segments[currentSegmentIdx]->addBlock(
                iNode, iMap.getNextINodeIndex());
    }
    iNodeAddress = (currentSegmentIdx << 10) + iNodeOffset;
    iMap.updateINodeAddressAtIndex(iNodeAddress, iNodeIndex);

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

void LFS::clean(unsigned numToClean) {
    unsigned numCleaned = 0;
    unsigned numFull = 0;
    while (numCleaned < numToClean || numFull == numToClean - 1) {
        unsigned emptyCount = 0;
        unsigned mostEmptyIdx = 0;
        for (unsigned i = 0; i < segments.size(); i++) {
            unsigned currentDeadCount = countDeadBlocksForSegmentAtIndex(i);
            if (emptyCount < currentDeadCount) {
                emptyCount = currentDeadCount;
                mostEmptyIdx = i;
            }
        }

        unsigned fillCount = 1024;
        unsigned mostFullIdx = 1;
        for (unsigned i = 0; i < segments.size(); i++) {
            unsigned currentDeadCount = countDeadBlocksForSegmentAtIndex(i);
            if (fillCount < currentDeadCount) {
                fillCount = currentDeadCount;
                mostFullIdx = i;
            }
        }

        combineSegments(mostFullIdx, mostEmptyIdx);
        if (segments[mostEmptyIdx]->isEmpty()) {
            numCleaned++;
        }
        if (segments[mostFullIdx]->emptyBlockCount() == 0) {
            numFull++;
        }
    }
}

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

void LFS::selectNewCleanSegment(bool recursion) {
    if (segments[currentSegmentIdx + 1]->isEmpty()) {
        // next segment is clean
        currentSegmentIdx += 1;
    } else {
        // find next clean segment to right
        bool foundClean = false;
        for (unsigned i = currentSegmentIdx; i < segments.size() && !foundClean; i++) {
            if (segments[i]->isEmpty()) {
                currentSegmentIdx = i;
                foundClean = true;
            }
        }
        // find clean segment to left
        for (unsigned i = 0; i < currentSegmentIdx && !foundClean; i++) {
            if (segments[i]->isEmpty()) {
                currentSegmentIdx = i;
                foundClean = true;
            }
        }
        // no clean segment, must clean then repeat process
        if (!recursion && !foundClean) {
            clean(32);
            selectNewCleanSegment(true);
        } else if (!foundClean) {
            // PANIC no clean segments and can't clean
        } else {
            // check remaining clean and clean if needed
        }
    }
}

void LFS::cleanSegmentAtIndex(unsigned index) {
    Segment* segment = segments[index];
    for (unsigned i = 0; i < 1024 - 8; i++) {
        unsigned currentBlockAddress = (index << 10) + i;

        unsigned blockIndexInINode = segment->getBlockStatusForBlockAtIndex(i);
        unsigned iNodeIndexInIMap = segment->getINodeStatusForBlockAtIndex(i);
        if (blockIndexInINode == std::numeric_limits<unsigned>::max()
                && iNodeIndexInIMap == std::numeric_limits<unsigned>::max()) {
            // block is empty
            continue;
        } else if (iNodeIndexInIMap == 10 * 1024) {
            // block is imap
            unsigned iMapAddress = iMapAddresses[blockIndexInINode];

            if (currentBlockAddress != iMapAddress) {
                segment->setBlockEmptyAtIndex(i);
            }
        } else if (blockIndexInINode == 128) {
            // block is inode
            unsigned iMapIndex = iNodeIndexInIMap % iMapAddresses.size();
            unsigned iMapAddress = iMapAddresses[iMapIndex]; 
            unsigned iMapSegmentIdx = getSegmentIndexFromAddress(iMapAddress);
            unsigned iMapBlockIdx = getBlockIndexFromAddress(iMapAddress);

            IMap iMap(segments[iMapSegmentIdx]->blocks[iMapBlockIdx]);
            unsigned iNodeAddress = iMap.iNodeAddresses[iNodeIndexInIMap];

            if (currentBlockAddress != iNodeAddress) {
                segment->setBlockEmptyAtIndex(i);
            }
        } else {
            // block is data
            unsigned iMapIndex = iNodeIndexInIMap % iMapAddresses.size();
            unsigned iMapAddress = iMapAddresses[iMapIndex];
            unsigned iMapSegmentIdx = getSegmentIndexFromAddress(iMapAddress);
            unsigned iMapBlockIdx = getBlockIndexFromAddress(iMapAddress);

            IMap iMap(segments[iMapSegmentIdx]->blocks[iMapBlockIdx]);
            unsigned iNodeAddress = iMap.iNodeAddresses[iNodeIndexInIMap];
            unsigned iNodeSegmentIdx = getSegmentIndexFromAddress(iNodeAddress);
            unsigned iNodeBlockIdx = getBlockIndexFromAddress(iNodeAddress);

            INode iNode(segments[iNodeSegmentIdx]->blocks[iNodeBlockIdx]);
            unsigned blockAddress = iNode.blockAddresses[blockIndexInINode];

            if (currentBlockAddress != blockAddress) {
                segment->setBlockEmptyAtIndex(i);
            }
        }
    }
}

void LFS::combineSegments(unsigned fullIndex, unsigned emptyIndex) {
    cleanSegmentAtIndex(fullIndex);
    cleanSegmentAtIndex(emptyIndex);
    Segment* fullSegment = segments[fullIndex];
    Segment* emptySegment = segments[emptyIndex];

    unsigned fullSegmentBlockIndex = 0;
    while (fullSegment->emptyBlockCount() != 3 && !emptySegment->isEmpty()) {
        while (!fullSegment->isEmptyAtIndex(fullSegmentBlockIndex)) {
            fullSegmentBlockIndex++;
        }

        unsigned emptySegmentBlockIndex = 0;
        while (!emptySegment->isEmptyAtIndex(emptySegmentBlockIndex)) {
            emptySegmentBlockIndex++;
        }

        unsigned blockIndexInINode =
            emptySegment->getBlockStatusForBlockAtIndex(emptySegmentBlockIndex);
        unsigned iNodeIndexInIMap =
            emptySegment->getINodeStatusForBlockAtIndex(emptySegmentBlockIndex);
        if (iNodeIndexInIMap == 10 * 1024) {
            // block is imap; just copy it
            IMap iMap(emptySegment->blocks[emptySegmentBlockIndex]);
            unsigned iMapAddress = (fullIndex << 10) + fullSegment->addBlock(iMap, blockIndexInINode);
            emptySegment->setBlockEmptyAtIndex(emptySegmentBlockIndex);
            iMapAddresses[blockIndexInINode] = iMapAddress;
        } else if (blockIndexInINode == 128) {
            // block is inode; copy it and update iMap
            unsigned iMapIndex = iNodeIndexInIMap % iMapAddresses.size();
            unsigned iMapAddress = iMapAddresses[iMapIndex]; 
            unsigned iMapSegmentIdx = getSegmentIndexFromAddress(iMapAddress);
            unsigned iMapBlockIdx = getBlockIndexFromAddress(iMapAddress);

            IMap iMap(segments[iMapSegmentIdx]->blocks[iMapBlockIdx]);

            INode iNode(emptySegment->blocks[emptySegmentBlockIndex]);

            unsigned iNodeAddress = (fullIndex << 10) + fullSegment->addBlock(iNode, iNodeIndexInIMap);
            emptySegment->setBlockEmptyAtIndex(emptySegmentBlockIndex);
            iMap.iNodeAddresses[iNodeIndexInIMap] = iNodeAddress;
            iMapAddress = (fullIndex << 10) + fullSegment->addBlock(iMap, iMapIndex);
            emptySegment->setBlockEmptyAtIndex(iMapBlockIdx);
            iMapAddresses[iMapIndex] = iMapAddress;
        } else {
            // block is data; copy all other data associated with the same file
            unsigned iMapIndex = iNodeIndexInIMap % iMapAddresses.size();
            unsigned iMapAddress = iMapAddresses[iMapIndex];
            unsigned iMapSegmentIdx = getSegmentIndexFromAddress(iMapAddress);
            unsigned iMapBlockIdx = getBlockIndexFromAddress(iMapAddress);

            IMap iMap(segments[iMapSegmentIdx]->blocks[iMapBlockIdx]);
            unsigned iNodeAddress = iMap.iNodeAddresses[iNodeIndexInIMap];
            unsigned iNodeSegmentIdx = getSegmentIndexFromAddress(iNodeAddress);
            unsigned iNodeBlockIdx = getBlockIndexFromAddress(iNodeAddress);

            INode iNode(segments[iNodeSegmentIdx]->blocks[iNodeBlockIdx]);

            unsigned currentIndex = emptySegmentBlockIndex;
            while (currentIndex < 1024 && fullSegment->emptyBlockCount() != 2) {
                unsigned currentBlockIndexInINode =
                    emptySegment->getBlockStatusForBlockAtIndex(currentIndex);
                unsigned currentINodeIndexInIMap =
                    emptySegment->getINodeStatusForBlockAtIndex(currentIndex);
                if (currentINodeIndexInIMap != iNodeIndexInIMap) {
                    continue;
                }

                unsigned blockAddress = 
                    (fullIndex << 10) + fullSegment->addBlock(emptySegment->blocks[currentIndex], currentBlockIndexInINode,
                    currentINodeIndexInIMap);
                emptySegment->setBlockEmptyAtIndex(currentIndex);
                iNode.updateBlockAddressAtIndex(blockAddress, currentBlockIndexInINode);

                currentIndex++;
            }

            iNodeAddress = (fullIndex << 10) + fullSegment->addBlock(iNode, iNodeIndexInIMap);
            emptySegment->setBlockEmptyAtIndex(iNodeBlockIdx);

            iMap.iNodeAddresses[iNodeIndexInIMap] = iNodeAddress;
            iMapAddress = (fullIndex << 10) + fullSegment->addBlock(iMap, iMapIndex);
            emptySegment->setBlockEmptyAtIndex(iMapBlockIdx);
            iMapAddresses[iMapIndex] = iMapAddress;
        }
    }
}

unsigned LFS::countDeadBlocksForSegmentAtIndex(unsigned index) {
    Segment* segment = segments[index];
    unsigned deadCount = 0;
    for (unsigned i = 0; i < 1024 - 8; i++) {
        unsigned currentBlockAddress = (index << 10) + i;

        unsigned blockIndexInINode = segment->getBlockStatusForBlockAtIndex(i);
        unsigned iNodeIndexInIMap = segment->getINodeStatusForBlockAtIndex(i);
        if (blockIndexInINode == std::numeric_limits<unsigned>::max()
                && iNodeIndexInIMap == std::numeric_limits<unsigned>::max()) {
            // block is empty
            deadCount++;
            continue;
        } else if (iNodeIndexInIMap == 10 * 1024) {
            // block is imap
            unsigned iMapAddress = iMapAddresses[blockIndexInINode];

            if (currentBlockAddress != iMapAddress) {
                deadCount++;
            }
        } else if (blockIndexInINode == 128) {
            // block is inode
            unsigned iMapIndex = iNodeIndexInIMap % iMapAddresses.size();
            unsigned iMapAddress = iMapAddresses[iMapIndex]; 
            unsigned iMapSegmentIdx = getSegmentIndexFromAddress(iMapAddress);
            unsigned iMapBlockIdx = getBlockIndexFromAddress(iMapAddress);

            IMap iMap(segments[iMapSegmentIdx]->blocks[iMapBlockIdx]);
            unsigned iNodeAddress = iMap.iNodeAddresses[iNodeIndexInIMap];

            if (currentBlockAddress != iNodeAddress) {
                deadCount++;
            }
        } else {
            // block is data
            unsigned iMapIndex = iNodeIndexInIMap % iMapAddresses.size();
            unsigned iMapAddress = iMapAddresses[iMapIndex];
            unsigned iMapSegmentIdx = getSegmentIndexFromAddress(iMapAddress);
            unsigned iMapBlockIdx = getBlockIndexFromAddress(iMapAddress);

            IMap iMap(segments[iMapSegmentIdx]->blocks[iMapBlockIdx]);
            unsigned iNodeAddress = iMap.iNodeAddresses[iNodeIndexInIMap];
            unsigned iNodeSegmentIdx = getSegmentIndexFromAddress(iNodeAddress);
            unsigned iNodeBlockIdx = getBlockIndexFromAddress(iNodeAddress);

            INode iNode(segments[iNodeSegmentIdx]->blocks[iNodeBlockIdx]);
            unsigned blockAddress = iNode.blockAddresses[blockIndexInINode];

            if (currentBlockAddress != blockAddress) {
                deadCount++;
            }
        }

    }
    return deadCount;
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

int LFS::findFile(std::string lfsFilename) {
    return files[lfsFilename];
}

