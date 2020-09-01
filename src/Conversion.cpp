#include "Conversion.h"


string decToHex(int num) {
	string ret = "";
	char c1 = '0', c2 = '0';
	const char digits[] = "0123456789ABCDEF";
	if (num>0) {
		while (num != 0) {
			c2 = digits[num % 16];
			num = num / 16;
			if (num != 0) {
				c1 = digits[num % 16];
				num = num / 16;
			}
			else c1 = '0';
			ret += c1;
			ret += c2; ret += " ";
		}
		return ret;
	}

	else if (num<0) {
		unsigned int n = num;
		while (n != 0) {
			c2 = digits[n % 16];
			n = n / 16;
			if (n != 0) {
				c1 = digits[n % 16];
				n = n / 16;
			}
			else c1 = '0';
			ret += c1;
			ret += c2; ret += " ";
		}
		return ret;
	}
	else {
		return "00 ";
		// return "00 00 ";
	}
}

string decToHexByte(int num) {
	if (num < -256 || num > 255) {
		cout << "Error: Too big number for byte type - " << num << ".";
		exit(-1);
	}
	string ins = decToHex(num);
	if (ins.length() > 3) ins.resize(3);
	return ins;
}

string decToHexWord(int num) {
	if (num < -32768 || num > 32767) {
		cout << "Error: Too big number for word type - " << num << ".";
		exit(-1);
	}
	string ins = decToHex(num);
	while (ins.length() < 6) {
		ins = ins + "00 ";
	}
	if (ins.length() > 6) ins.resize(6);
	return ins;
}

int hexToDec(string hexNr) {
	stringstream ss; unsigned int res = 0;
	ss << hex << hexNr;
	ss >> res;
	return res;
}


bool startsWith(const string& s, const string& prefix) {
	return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}


int contains(const vector<string>& strs, const string& str) {
	int ind = 0;
	for (string s : strs) {
		if (s == str) return ind;
		ind++;
	}
	return -1;
}

vector<string> split(string s, string delimiter) {
	size_t pos_start = 0, pos_end, delim_len = delimiter.length();
	string token;
	vector<string> res;

	while ((pos_end = s.find(delimiter, pos_start)) != string::npos) {
		token = s.substr(pos_start, pos_end - pos_start);
		pos_start = pos_end + delim_len;
		res.push_back(token);
	}

	res.push_back(s.substr(pos_start));
	return res;
}

string toLower(const string& s) {
	string s1 = "";
	for (char c : s) {
		s1 += tolower(c);
	}
	return s1;
}

string toUpper(const string& s) {
	string s1 = "";
	for (char c : s) {
		s1 += toupper(c);
	}
	return s1;
}