#ifndef _LFS_H_
#define _LFS_H_

#include <istream>
#include <string>
#include <map>
#include <vector>

class LFS {
    private:
        std::map<std::string, int> files;
        std::vector<unsigned int> checkpoint;

    public:
        LFS();

        void import(std::string lfsFilename, std::istream &data);
        std::string list();
        void remove(std::string lfsFilename);
        //std::string cat(std::string lfsFilename);
        //std::string display(std::string lfsFilename, int howMany, int start);
        //void overwrite(std::stirng lfsFilename, int howMany, int start, char c);
        void flush();
        //void clean();
};

#endif
