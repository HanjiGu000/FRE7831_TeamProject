//
//  Util.cpp
//  Team_Project
//
//  Created by Hanji Gu on 4/23/22.
//

#include "Util.hpp"
#include <vector>
#include <fstream>
#include <sstream>
using namespace std;


vector<string> split(string text, char delim) {
    string line;
    vector<string> vec;
    stringstream ss(text);
    while (std::getline(ss, line, delim)) {
            vec.push_back(line);
    }
    return vec;
}


vector<pair<string, string>> GetPairs(string input_file)
{
    vector<pair<string, string>> pair_map;
    ifstream fin;
    fin.open(input_file, ios::in);
    string line, stock1, stock2;
    while (!fin.eof())
    {
        getline(fin, line);
        line.erase(remove(line.begin(), line.end(), '\r'), line.end());
        stringstream sin(line);
        getline(sin, stock1, ',');
        getline(sin, stock2);
        pair_map.push_back(pair<string, string>(stock1, stock2));
    }
    return pair_map;
}
