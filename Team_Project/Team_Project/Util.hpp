//
//  Util.hpp
//  Team_Project
//
//  Created by Hanji Gu on 4/23/22.
//

#ifndef Util_hpp
#define Util_hpp

#include <stdio.h>
#include <map>
#include <string>
#include <iostream>
#include <ctime>
using namespace std;

vector<string> split(string text, char delim);

vector<pair<string, string>> GetPairs(string input_file);

map<string, string> ProcessConfigData(string config_file);

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

string GetCurrentDate();
#endif /* Util_hpp */
