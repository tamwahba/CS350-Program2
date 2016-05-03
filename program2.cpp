#include <iostream>
#include <cstring>
#include <ctime>
#include <cstdlib>

using namespace std;

void print_usage()
{

    cout<<"Usage:\n"<<endl;
    cout<<"program2 LFS"<<endl;
    cout<<"\n"<<endl;
    cout<<"-U     Print this Usage\n"<<endl;
    cout<<"\n"<<endl;
    cout<<"import <filename> <lfs_filename> \n"<<endl;
    cout<<"remove <lfs_filename>\n"<<endl;
    cout<<"cat <lfs_filename>\n"<<endl;
    cout<<"display <lfs_filename> <howmany> <start>\n"<<endl;
    cout<<"overwrite <lfs_filename> <howmany> <start> <c>\n"<<endl;
    cout<<"list\n"<<endl;
    cout<<"exit\n"<<endl;
    cout<<"\n"<<endl;

}

int main(int argc, char* argv[]) 
{

    while (true)
    {

        std::string command;

        std::cin >> command;

        if (command.size() == 0)
            continue;

        ///run the command here
        if (command == "import") {
            std::string filename, lfs_filename;

            std::cin >> filename >> lfs_filename;

        }
         else if(command == "remove"){
            std::string lfs_filename;
        }

        else if (command == "cat") {
            std::string lfs_filename;

            std::cin >> lfs_filename;

        } 

        else if (command == "display") {
            std::string lfs_filename;
            int howmany;
            int start;

            std::cin >> lfs_filename;

        }

        else if (command == "overwrite") {
            std::string lfs_filename;
            int howmany;
            int start;
            int c;

            std::cin >> lfs_filename;
            cin>> howmany>>start>>c;

        }

        if (command == "exit")
        {
            exit(0);
        } 
        else if (command == "list") {
            ///do that
        } else {
            std::cerr << "Invalid command: " << command << std::endl;
                break;
        }
    }

    return 0;
}