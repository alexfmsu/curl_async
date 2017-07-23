#ifndef FILEDOWNLOADER_HPP
#define FILEDOWNLOADER_HPP

#include <iostream>

using namespace std;

std::string basename(const std::string &url){
    size_t found = url.find_last_of ("/\\");
    
    return url.substr(found + 1);
}

size_t static write_data(void *ptr, size_t size, size_t nmemb, FILE* stream){
    size_t written = fwrite(ptr, size, nmemb, stream);
        
    return written;
}

class FileDownloader{

public:
    FileDownloader(){
        curl_global_init(CURL_GLOBAL_ALL);
        
        multi_handle = curl_multi_init();
        
        job_id = 0;
        running_jobs = -1;
    }
    
    int run(std::vector<std::string> urls){
        curl_multi_setopt(multi_handle, CURLMOPT_MAXCONNECTS, max_jobs);
        
        for(job_id = 0; job_id < max_jobs; job_id++){
            if(job_id >= urls.size()){
                break;
            }

            add_job(multi_handle, urls[job_id]);
        }
        
        while(running_jobs){
            curl_multi_perform(multi_handle, &running_jobs);
            
            if(running_jobs){
                FD_ZERO(&read_fd);
                FD_ZERO(&write_fd);
                FD_ZERO(&exec_fd);
                
                if(curl_multi_fdset(multi_handle, &read_fd, &write_fd, &exec_fd, &max_fd)){
                    cout << "Error" << endl;
                    
                    return -1;
                }
                
                if(curl_multi_timeout(multi_handle, &timeout)){
                    cout << "Error: timeout exceeded" << endl;
                    
                    return -1;
                }
                
                if(max_fd == -1){
                    sleep((unsigned int) timeout / 1000);
                }else{
                    struct timeval fd_timeout = {timeout / 1000, (timeout % 1000) * 1000};
                    
                    if(select (max_fd + 1, &read_fd, &write_fd, &exec_fd, &fd_timeout) < 0){
                        cout << "Error" << endl;
                        
                        return -1;
                    }
                }
            }
            
            while((msg = curl_multi_info_read(multi_handle, &msgs_in_queue))){
                if(msg->msg == CURLMSG_DONE){
                    char *url;
                    double size;
                    
                    CURL *curl = msg->easy_handle;
                    
                    curl_easy_getinfo(curl, CURLINFO_PRIVATE, &url);
                    curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &size);
                    
                    printf("\x1b[32mSuccess\x1b[0m: %s for <%s>\n", curl_easy_strerror (msg->data.result), url);
                    printf("\tDownloaded size: %.0f bytes\n", size);
                    
                    curl_multi_remove_handle(multi_handle, curl);
                    curl_easy_cleanup(curl);
                }else{
                    cout << "Error" << endl;
                }
                
                if(job_id < urls.size()){
                    add_job(multi_handle, urls[job_id++]);
                    
                    running_jobs++;
                }
            }
        }
        
        return 0;
    }
    
    void set_timeout(long int _timeout);
    
    void set_max_jobs(int);
    
    static void add_job(CURLM* multi_handle, const std::string &url){
        CURL *curl = curl_easy_init();
        
        curl_easy_setopt(curl, CURLOPT_HEADER, 0L);
        curl_easy_setopt(curl, CURLOPT_PRIVATE, url.c_str());
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fopen(basename(url).c_str(), "wb"));
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        
        curl_multi_add_handle (multi_handle, curl);
    }
    
    ~FileDownloader(){
        curl_multi_cleanup(multi_handle);
        
        curl_global_cleanup();
    }

private:
    CURLM *multi_handle;
    
    CURL* curl;
    
    unsigned int job_id;
    long max_jobs;
    
    int max_fd, msgs_in_queue, running_jobs;
    
    CURLMsg *msg;
    fd_set read_fd, write_fd, exec_fd;
    
    long int timeout;
};

#endif