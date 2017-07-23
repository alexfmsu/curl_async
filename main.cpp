#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <unistd.h>
#include <curl/multi.h>

#include "FileDownloader.cpp"

using namespace std;

int main(int argc, char** argv){
    if(argc < 2){
        cout << "Usage: ./main.x <url_1 .. url_N>" << endl;
        
        return 0;
    }
    
    FileDownloader downloader;
    downloader.set_timeout(100);
    downloader.set_max_jobs(15);
    
    std::vector<std::string> urls(argv + 1, argv + argc);
    
    downloader.run(urls);
    
    return 0;
}