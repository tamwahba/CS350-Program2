#ifndef _LFS_H_
#define _LFS_H_

#include "Segment.h"
#include "IMap.h"
#include "INode.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>

class LFS {
    private:
        std::map<std::string, int> files;
        std::vector<Segment> segments;
        std::vector<unsigned int> checkpoint;

    public:
        LFS();

        void import(std::string lfsFilename, std::string data);
        std::string list();
        void remove(std::string lfsFilename);
        //std::string cat(std::string lfsFilename);
        //std::string display(std::string lfsFilename, int howMany, int start);
        //void overwrite(std::string lfsFilename, int howMany, int start, char c);
        void flush();
        //void clean();
};

#endif
