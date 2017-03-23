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
#include "parse_config.h"
#include <ctime> 

using namespace std;

Queue<string> producerQueue;
Queue<string> consumerQueue;
pthread_cond_t consumer_cond = PTHREAD_COND_INITIALIZER; 
pthread_cond_t producer_cond = PTHREAD_COND_INITIALIZER;

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

string fetch_webpage(string site_name) {
  CURL *curl_handle;
  CURLcode res;
  const char *site = site_name.c_str();
  struct MemoryStruct chunk;
 
  chunk.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */ 
  chunk.size = 0;    /* no data at this point */ 
 
  curl_global_init(CURL_GLOBAL_ALL);
 
  /* init the curl session */ 
  curl_handle = curl_easy_init();
 
  /* specify URL to get */ 
  curl_easy_setopt(curl_handle, CURLOPT_URL, site);
 
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

int parse_data(string site_data, string search_string) { 
  size_t start_pos = 0; 
  int count = 0;
  while (start_pos != string::npos) { // keep checking each time it's found
    start_pos = site_data.find(search_string, start_pos+1); 
    if (start_pos != string::npos) { // only increase count if found 
      count++; 
    }
  }
  return count; 
}

string get_time() {
  time_t time1; 
  struct tm *time_info; 

  time (&time1); 
  time_info = localtime(&time1);
  char * ctime = asctime(time_info);
  ctime[strlen(ctime) -1] = '\0'; 
  string mytime = ctime;
  return mytime;
}
 
void *threadFetch(void * pData) {
  while(1) {
    //condition_variable; 
    pthread_mutex_lock(&producerQueue.lock);
    while (producerQueue.empty()) 
      pthread_cond_wait(&producer_cond, &producerQueue.lock); 
    string site = producerQueue.pop(); 
    pthread_mutex_unlock(&producerQueue.lock);
    string output = fetch_webpage(site); 
    pthread_mutex_lock(&consumerQueue.lock);
    consumerQueue.push(output); 
    pthread_cond_signal(&consumer_cond);
    pthread_mutex_unlock(&consumerQueue.lock);
  }
}

int main(int argc, char *argv[]) {

  string output;
  Parse p;
  p.parse_config(argv[1]); // get all of config variables 
  p.parse_site_file(); // create vector of sites 
  p.parse_search_file(); // create vector of search terms 

  pthread_mutex_lock(&producerQueue.lock); 
  for (unsigned int i=0; i<p.get_sites_size(); i++) { // add every site to producerQueue
    producerQueue.push(p.get_site(i));
    pthread_cond_signal(&producer_cond);
  }
  pthread_mutex_unlock(&producerQueue.lock);

  for (unsigned int i = 0; i < p.get_sites_size(); i++) {
    output = fetch_webpage(producerQueue.pop()); // fetch data from site and pop it off
    consumerQueue.push(output); // add output from fetch to consumerQueue 
  }
  
  int fetch_num = 1; 
  int count; 
  // this currently does the 5% off thing, which I didn't realize I did so it'll need to be fixed
  while (!consumerQueue.empty()) { // parse every site output in consumerQueue
      ofstream output_file; 
      string file_name = to_string(fetch_num) + ".csv"; 
      output_file.open(file_name);
      for (unsigned int i = 0; i < p.get_searches_size(); i++) { // search for each search term
        string search_term = p.get_search_term(i); 
        string site = p.get_site(fetch_num-1); // this might be a hack that won't work later
        count = parse_data(consumerQueue.front(), search_term);
        string time = get_time();  
        output_file << time + "," + site + "," + search_term + "," + to_string(count) + "\n"; 
      }
      consumerQueue.pop(); // pop off completely searched site data 
      fetch_num++; // next batch 
      output_file.close();
  }
  return 0;
}