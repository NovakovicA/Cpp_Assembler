#ifndef CONVERSION_CPP
#define CONVERSION_CPP
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <regex>
#include <vector>
#include <cmath> 

using namespace std;

string decToHex(int);
string decToHexByte(int);
string decToHexWord(int);

int hexToDec(string);

bool startsWith(const string& s, const string& prefix);

int contains(const vector<string>& strs, const string& str);

vector<string> split(string s, string delimiter);

string toLower(const string& s);

string toUpper(const string& s);



#endif