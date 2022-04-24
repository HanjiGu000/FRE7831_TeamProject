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


map<string, string> ProcessConfigData(string config_file)
{
    map<string, string> config_map;
    ifstream fin;
    fin.open(config_file, ios::in);
    string line, name, value;
    while (!fin.eof())
    {
        getline(fin, line);
        // std::remove: Transforms the range [first,last) into a range with all the elements
        // that compare equal to val removed, and returns an iterator to the new
        // end of that range.
        // string::erase: Erases part of the string, reducing its length
        line.erase(remove(line.begin(), line.end(), '\r'), line.end());
        stringstream sin(line);
        getline(sin, name, ':');
        getline(sin, value);
        cout << value << endl;
        config_map.insert(pair<string, string>(name, value));
    }
    return config_map;
}


size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}


string GetCurrentDate()
{
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];

    time (&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, 80, "%Y-%m-%d", timeinfo);
    string str(buffer);

    cout << str << endl;;

    return str;
}
