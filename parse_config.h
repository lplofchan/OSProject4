/* Operating Systems Project 4
Leah Plofchan and Brynna Conway */
 
#ifndef _PARSE_
#define _PARSE_

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>

using namespace std;

class Parse {
    public:
        Parse();        // constructor
        void parse_config(char *config);
        void parse_site_file();
        void parse_search_file();
        string get_site(unsigned int i); 
        unsigned int get_sites_size();
        unsigned int get_searches_size();
        string get_search_term(unsigned int i); 

    private:
        int PERIOD_FETCH;
        int NUM_FETCH;
        int NUM_PARSE;
        string SEARCH_FILE;
        string SITE_FILE;
        vector<string> sites;
        vector<string> searches;
};

Parse::Parse() {
    int PERIOD_FETCH = 180;
    int NUM_FETCH = 1;
    int NUM_PARSE = 1;
    string SEARCH_FILE = "Search.txt";
    string SITE_FILE = "Sites.txt";
}

void Parse::parse_config(char *config) {
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
          }
          else if (tokens[0] == "NUM_PARSE") {
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

void Parse::parse_site_file() {
    ifstream myfile;
    myfile.open(SITE_FILE);
    string line;
    if (myfile.is_open()) {
        while (getline(myfile, line)) {
            sites.push_back(line);
        }
    }
}

void Parse::parse_search_file() {
    ifstream myfile;
    myfile.open(SEARCH_FILE);
    string line;
    if (myfile.is_open()) {
        while (getline(myfile, line)) {
            searches.push_back(line);
        }
    }
}

string Parse::get_site(unsigned int i) {
    return sites[i];
}

unsigned int Parse::get_sites_size() {
    return sites.size();
}

unsigned int Parse::get_searches_size() {
    return searches.size();
}

string Parse::get_search_term(unsigned int i) {
    return searches[i];
}

#endif