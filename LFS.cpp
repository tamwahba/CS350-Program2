#include "LFS.h"

LFS::LFS() {
    current = 0;
    numClean = 0;
    blockIndex = 0;
    isClean.resize(32, 1);
    //Construct each segment
    mkdir("DRIVE",  S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    for(int i = 0; i < 32; i++) {
        std::cout << "Constructing segment " << i + 1 << std::endl;
        segments.push_back(Segment());
        std::fstream SEGMENT("DRIVE/SEGMENT" + std::to_string(i + 1), std::ios::binary | std::ios::in | std::ios::out);
        std::cout << "Passing data to segment " << i << std::endl;
        if(SEGMENT.peek() == std::ifstream::traits_type::eof()) {
            SEGMENT.close();
            std::ofstream SEGMENT2("DRIVE/SEGMENT" + std::to_string(i + 1), std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc); 
            for(int j = 0; j < 32 * 1024; j++) {
                SEGMENT2.put('\0');
            }
            SEGMENT2.close();
            SEGMENT.open("DRIVE/SEGMENT" + std::to_string(i + 1), std::ios::binary | std::ios::in | std::ios::out);
        }
        SEGMENT >> segments[i];
        SEGMENT.close();
    }

    std::cout << "Constructing the CR" << std::endl;
    //Construct the CR
    std::ifstream CHECKPOINT_REGION("DRIVE/CHECKPOINT_REGION", std::ios::in | std::ios::binary);
    if(CHECKPOINT_REGION.peek() == std::ifstream::traits_type::eof()) {
        CHECKPOINT_REGION.close();
        std::ofstream CHECKPOINT_REGION2("DRIVE/CHECKPOINT_REGION", std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc); 
        for(int i = 0; i < 192; i++) {
            CHECKPOINT_REGION2.put('\0');
        }
        CHECKPOINT_REGION2.close();
        CHECKPOINT_REGION.open("DRIVE/CHECKPOINT_REGION", std::ios::binary | std::ios::in | std::ios::out);
    }

    unsigned int tempIndex = 0;
    for(int i = 0; i < 32 && CHECKPOINT_REGION.read((char*)&tempIndex, sizeof(unsigned)); i++) {
        isClean[i] = tempIndex;
        std::cout << tempIndex << std::endl;
    }
    while(CHECKPOINT_REGION.read((char*)&tempIndex, sizeof(unsigned))) {
        if (tempIndex > 0) {
            checkpoint.push_back(tempIndex);
        }
    }
    CHECKPOINT_REGION.close();
    std::cout << "Constructing the file map" << std::endl;
    //Construct the file map
    for(auto i: checkpoint) {
        IMap* iMap = static_cast<IMap*>(getBlock(i));
        for(auto j: iMap->iNodes) {
            INode* iNode = static_cast<INode*>(getBlock(j));
            std::string fileName = iNode->fileName;
            files[fileName] = j;
        }
    }
    updateClean();
    std::cout << "Finished constructing LFS" << std::endl;
}

void LFS::updateClean() {
    bool found = false;
    for(unsigned int i = current; i < 32 && !found; i++) {
        if(isClean[i]) current = i;
        found = true;
    }
    for(unsigned int i = 0; i < current && !found; i++) {
        if(isClean[i]) current = i;
        found = true;
    }
}

Block* LFS::getBlock(unsigned int address) {
    return segments[address >> 10].blocks[address & 0x3FF];
}

void LFS::import(std::string lfsFilename, std::istream& data) {
    INode* iNode = new INode();
    iNode->fileName = lfsFilename;
    Block block;
    data >> block;
    std::cout << "BlockIndex: " << blockIndex << std::endl;
    unsigned int address = (current << 10) + blockIndex;
    do {
        iNode->fileSize++;     
        address = (current << 10) + blockIndex;
        iNode->blockIndices[blockIndex] = address;
        blockIndex++;
        
        bool remSpace = segments[current].addBlock(block, 2);
        if(remSpace) continue;
        
        std::cout << "Section 2" << std::endl;
        segments[current].addBlock(*iNode, 1);
        files[lfsFilename] = address + 1;
        IMap* iMap = new IMap();
        if((files.size() / 256) < checkpoint.size()) {
            unsigned int iMapAddress = checkpoint[files.size() / 256];
            iMap = static_cast<IMap*>(getBlock(iMapAddress));
        } else {
            checkpoint.push_back(0);
            iMap->iNodes.push_back(0);
        }
        iMap->iNodes[files.size() % 256] = address + 1;
        segments[current].addBlock(*iMap, 0);
        checkpoint[files.size() / 256] = address + 2;
        flush();

        blockIndex = 0;
        updateClean();
        isClean[current] = 0;
    } while(data >> block);

    segments[current].addBlock(*iNode, 1);
    files[lfsFilename] = address + 1;
    IMap* iMap = new IMap();
    if((files.size() / 256) < checkpoint.size()) {
        unsigned int iMapAddress = checkpoint[files.size() / 256];
        iMap = static_cast<IMap*>(getBlock(iMapAddress));
    } else {
        checkpoint.push_back(0);
        iMap->iNodes.push_back(0);
    }
    iMap->iNodes[files.size() % 256] = address + 1;
    segments[current].addBlock(*iMap, 0);
    checkpoint[files.size() / 256] = address + 2;
    flush();
    blockIndex += 2;
}

std::string LFS::list() {
    //For each INode in
    std::stringstream list;
    std::cout << "Number of files: " << files.size() << std::endl;
    for(auto& i: files) {
        std::cout << "File " << i.second << ": " << std::endl;
        INode* iNode = static_cast<INode*>(getBlock(i.second));
        list << iNode->fileName << " " << iNode->fileSize << std::endl;
    }
    return list.str();
}

void LFS::remove(std::string lfsFilename) {
    //Find and erase the INode pointer of the file in the IMap
    unsigned int iNodeLoc = files[lfsFilename];
    bool found = false;
    //each IMap piece
    for(unsigned int i = 0; i < checkpoint.size() && !found; i++) { 
        IMap* iMap = static_cast<IMap*>(getBlock(i));
        //each INode
        for(unsigned int j = 0; j < iMap->iNodes.size() && !found; j++) {
            if(iMap->iNodes[j] == iNodeLoc) {
                iMap->iNodes.erase(iMap->iNodes.begin() + j);
                found = true;
            }
        }
    }

    //Remove the file from the files map
    files.erase(lfsFilename);
}

/*std::string LFS::cat(std::string lfsFilename) {


  }*/

/*std::string LFS::display(std::string lfsFilename, int howMany, int start) {


  }*/

/*void LFS::overwrite(std::string lfsFileName, int howMany, int start, char c) {


  }*/

void LFS::flush() {
    for(int i = 0; i < 32; i++) {
        std::ofstream SEGMENT("DRIVE/SEGMENT" + std::to_string(i + 1), std::ios::out | std::ios::trunc | std::ios::binary);
        SEGMENT << segments[i];
        SEGMENT.close();
    }
    std::ofstream CHECKPOINT_REGION("DRIVE/CHECKPOINT_REGION", std::ios::out | std::ios::trunc | std::ios::binary);
    for(auto i: isClean) {
        std::cout << "WRITING CLEAN: " << i << std::endl; 
        CHECKPOINT_REGION.write((char*)&i, sizeof(unsigned));
    }
    for(auto c: checkpoint) {
        CHECKPOINT_REGION.write((char*)&c, sizeof(unsigned));
    }
    CHECKPOINT_REGION.close();
}

/*void LFS::clean() {


  }*/

