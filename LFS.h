#ifndef _LFS_H_
#define _LFS_H_

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>

#include "Segment.h"
#include "IMap.h"
#include "INode.h"

class LFS {
    public:
        LFS();

        void import(std::string lfsFilename, std::istream &data);
        std::string list();
        void remove(std::string lfsFilename);
        //std::string cat(std::string lfsFilename);
        //std::string display(std::string lfsFilename, int howMany, int start);
        //void overwrite(std::string lfsFilename, int howMany, int start, char c);
        void flush();
        //void clean();

    private:
        std::map<std::string, unsigned> files;
        // std::vector<Segment> segments;
        // std::vector<unsigned int> checkpoint;
        // std::vector<unsigned int> isClean; 
        unsigned currentSegmentIdx;
        unsigned currentBlockIdx;
        unsigned numCleanSegmanets;

        unsigned getBlockIndexFromAddress(unsigned address);
        unsigned getSegmentIndexFromAddress(unsigned address);
        void updateClean();
};

#endif
