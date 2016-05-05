#ifndef _LFS_H_
#define _LFS_H_

#include "Segment.h"
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>

class LFS {
    private:
        std::map<std::string, int> files;
        std::vector<Segment*> segments;
        std::vector<unsigned int> checkpoint;
        std::vector<unsigned int> isClean; 
        unsigned int current;
        unsigned int numClean;
        unsigned int blockIndex;

        void updateClean();
        Block* getBlock(unsigned int address);

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
};

#endif
