#include "LFS.h"

LFS::LFS() {
    //Construct each segment
    for(int i = 0; i < 32; i++) {
        segments.push_back(Segment());
        std::ifstream SEGMENT("DRIVE/SEGMENT" + std::to_string(i), std::ios::in);
        SEGMENT >> segments[i];
    }
    
    //Construct the CR
    std::ifstream CHECKPOINT_REGION("DRIVE/CHECKPOINT_REGION", std::ios::in);
    std::string tempLine;
    while(getline(CHECKPOINT_REGION, tempLine)) checkpoint.push_back(std::stoul(tempLine));
    if(checkpoint.empty()) {
        segments[0].blocks[0] = IMap();
        checkpoint.push_back(0);
    }

    //Construct the file map
    for(auto i: checkpoint) {
        IMap* iMap = static_cast<IMap*>(&segments[i >> 10].blocks[i && 0x3FF]);
        for(auto j: iMap->iNodes) {
            INode* iNode = static_cast<INode*>(&segments[j >> 10].blocks[j && 0x3FF]);
            std::string fileName = iNode->fileName;
            files[fileName] = j;
        }
    }
}

void LFS::import(std::string lfsFilename, std::string data) {
   //unsigned int numDataBlocks = data.size.length() / 1024; 
}

std::string LFS::list() {
    //For each INode in
    std::stringstream list;
    for(auto& i: files) {
        INode* iNode = static_cast<INode*>(&segments[i.second >> 10].blocks[i.second && 0x3FF]);
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
        IMap* iMap = static_cast<IMap*>(&segments[i >> 10].blocks[i && 0x3FF]);
        //each INode
        for(unsigned int j = 0; j < iMap->iNodes.size() && !found; j++) {
            if(iMap->iNodes[j] ==iNodeLoc) {
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
        std::ofstream SEGMENT("DRIVE/SEGMENT" + std::to_string(i), std::ios::out);
        SEGMENT << segments[i];
    }
}

/*void LFS::clean() {


}*/

