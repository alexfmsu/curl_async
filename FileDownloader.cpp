#ifndef FILEDOWNLOADER_CPP
#define FILEDOWNLOADER_CPP


#include "FileDownloader.hpp"

void FileDownloader::set_timeout(long int _timeout){
    timeout = _timeout;
}

void FileDownloader::set_max_jobs(int max_conn){
    max_jobs = max_conn;
}

#endif