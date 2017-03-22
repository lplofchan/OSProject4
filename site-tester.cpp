/* Operating Systems Project 4
Leah Plofchan and Brynna Conway */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <curl/curl.h>
#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>
#include "Queue.h"
#include "parse.h"

using namespace std;


Queue<string> fetchQueue;
Queue<string> parseQueue;

struct MemoryStruct {
  char *memory;
  size_t size;
};
 
static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;
 
  mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL) {
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }
 
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
 
  return realsize;
}

string fetch_webpage() {
  CURL *curl_handle;
  CURLcode res;
 
  struct MemoryStruct chunk;
 
  chunk.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */ 
  chunk.size = 0;    /* no data at this point */ 
 
  curl_global_init(CURL_GLOBAL_ALL);
 
  /* init the curl session */ 
  curl_handle = curl_easy_init();
 
  /* specify URL to get */ 
  curl_easy_setopt(curl_handle, CURLOPT_URL, "https://www.nd.edu/");
 
  /* send all data to this function  */ 
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
 
  /* we pass our 'chunk' struct to the callback function */ 
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
 
  /* some servers don't like requests that are made without a user-agent
     field, so we provide one */ 
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
 
  /* get it! */ 
  res = curl_easy_perform(curl_handle);
 
  /* check for errors */ 
  if(res != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
  }

  /* cleanup curl stuff */ 
  curl_easy_cleanup(curl_handle);
  string output = chunk.memory; 
 
  free(chunk.memory);
 
  /* we're done with libcurl, so clean it up */ 
  curl_global_cleanup();
 
  return output;

}

void parse_config(char* config) {
    int PERIOD_FETCH = 180;
    int NUM_FETCH = 1;
    int NUM_PARSE = 1;
    string SEARCH_FILE = "Search.txt";
    string SITE_FILE = "Sites.txt";
    ifstream myfile;
    myfile.open(config);
    string line;
    //stringstream ss;
    string item;
    vector<string> tokens;
    if (myfile.is_open()) {
        while (getline(myfile, line)) {
          stringstream ss(line);
          //ss.str(line);
          while (getline(ss,item, '=')) {
            tokens.push_back(item);
            //cout << item << endl;
          }
          if (tokens[0] == "PERIOD_FETCH") {
              PERIOD_FETCH = stoi((tokens[1]));
          }
          else if (tokens[0] == "NUM_FETCH") {
              NUM_FETCH = stoi((tokens[1]));
          }else if (tokens[0] == "NUM_PARSE") {
              NUM_PARSE = stoi((tokens[1]));
          }

          else if (tokens[0] == "SEARCH_FILE") {
              SEARCH_FILE = (tokens[1]);
          }
          else if (tokens[0] == "SITE_FILE") {
              SITE_FILE = (tokens[1]);
          }
          else if (tokens[0] != "1") {
              cout << "Unknown paramater" << endl;
          }
          tokens.clear();
        }
        myfile.close();
    }
    else {
        cout << "unable to open file" << endl;
    }

}
 
int main(int argc, char *argv[])
{
  cout << fetch_webpage() << endl;
  
  Parse p;
  p.parse_config(argv[1]);
  p.parse_search_file();

}