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
#include <tuple> 
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

using namespace std;

/* TO DO
- SIGNAL/TIMER
- GET 1 THREAD WORKING
- GET MULTIPLE THREADS WORKING
- CAPTURE EXIT ON CTRL-C
- README
- CORRECT OPERATION WHEN SITES HAVE ERRORS (TIMEOUTS)
- PROTECTION AGAINST CONFIGURATION ERRORS
 */

Queue<string> producerQueue;
Queue<tuple <string, string, string> > consumerQueue;
pthread_cond_t consumer_cond = PTHREAD_COND_INITIALIZER; 
pthread_cond_t producer_cond = PTHREAD_COND_INITIALIZER;
Parse p;
ofstream output_file;
int keeplooping = 1;

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
  CURLcode res;
  const char *site = site_name.c_str();
  struct MemoryStruct chunk;
  CURL *curl_handle;
  chunk.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */ 
  chunk.size = 0;    /* no data at this point */ 
 
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
 
void * threadFetch(void * pData) {
  while(1) {
    //condition_variable; 
    
    pthread_mutex_lock(&producerQueue.lock);
    while (producerQueue.empty()) 
      pthread_cond_wait(&producer_cond, &producerQueue.lock); 
    string site = producerQueue.pop(); 
    pthread_mutex_unlock(&producerQueue.lock);
    string output = fetch_webpage(site);
    auto output_data = make_tuple(site, output, get_time());
    pthread_mutex_lock(&consumerQueue.lock);
    consumerQueue.push(output_data); 
    pthread_cond_broadcast(&consumer_cond);
    pthread_mutex_unlock(&consumerQueue.lock);
  }
}

void * threadParse(void * pData) {
  while(1) {
    pthread_mutex_lock(&consumerQueue.lock);
    while (consumerQueue.empty()) {
      pthread_cond_wait(&consumer_cond, &consumerQueue.lock);
      
    }
    auto site_data = consumerQueue.pop();
    for (unsigned int i = 0; i < p.get_searches_size(); i++) { // search for each search term
        string search_term = p.get_search_term(i); 
        cout << "search term: " << search_term << endl;
        string site = get<0>(site_data);
        cout << "site: " << site << endl;
        string output = get<1>(site_data);
        string my_time = get<2>(site_data);
        int count = parse_data(output, search_term);
        cout << "count: " << count << endl;
        output_file << my_time + "," + site + "," + search_term + "," + to_string(count) + "\n"; 
        cout << "output: " << output_file << endl;
   }
    pthread_mutex_unlock(&consumerQueue.lock);
  }
}

void ExitHandler(int) {
     keeplooping = 0;
     //pthread_cond_broadcast(&producer)
     output_file.close();
     exit(1);
}


int main(int argc, char *argv[]) {
  string output;
  p.parse_config(argv[1]); // get all of config variables 
  p.parse_site_file(); // create vector of sites 
  p.parse_search_file(); // create vector of search terms 
  pthread_t *fetch_threads = (pthread_t *)malloc(sizeof(pthread_t) * p.get_num_fetch());
  pthread_t *parse_threads = (pthread_t *)malloc(sizeof(pthread_t) * p.get_num_parse());
  int batch_num = 1;
  signal(SIGINT, ExitHandler);

  curl_global_init(CURL_GLOBAL_ALL);
  /* init the curl session */ 
  while (keeplooping) {
    string file_name = to_string(batch_num) + ".csv"; 
    output_file.open(file_name);

    // put this stuff in while loop eventually
    pthread_mutex_lock(&producerQueue.lock); 
    
    for (int i=0; i< p.get_num_fetch(); i++) {
      pthread_create(&fetch_threads[i], NULL, threadFetch, NULL);
    }
    

    for (int i=0; i< p.get_num_parse(); i++) {
      pthread_create(&parse_threads[i], NULL, threadParse, NULL);
    }
    for (unsigned int i=0; i<p.get_sites_size(); i++) { // add every site to producerQueue
      producerQueue.push(p.get_site(i));
      pthread_cond_signal(&producer_cond);
    }
      
    pthread_mutex_unlock(&producerQueue.lock);

    curl_global_cleanup();
    for (int i = 0; i < p.get_num_fetch(); i++) {		// wait for threads to complete and join all threads together
		  if (pthread_join(fetch_threads[i], NULL) != 0) {
			  fprintf(stderr, "ERROR: %s\n", strerror(errno));
		  }
	  }

    for (int i = 0; i < p.get_num_parse(); i++) {		// wait for threads to complete and join all threads together
		  if (pthread_join(parse_threads[i], NULL) != 0) {
			  fprintf(stderr, "ERROR: %s\n", strerror(errno));
		  }
	  }

    sleep(p.get_period());
    output_file.close();
    batch_num++;
  }

  return 0;
}