#ifndef SYMBOL_H
#define SYMBOL_H

#include <iostream>
#include <string>
#include "forwardRefs.h"
#include <map>

using namespace std;

class Symbol{
	string name;
	string section;
	int offset;
	string type;
	bool defined;
	string data = "";
	string relocationRecord = "";

	//Used for EQU
	map<string, int> IoC;
public:
	int numOfPatches = 0;
	Symbol(string name, string section, int offset, string type, bool defined) {
		this->name = name;
		this->section = section;
		this->offset = offset;
		this->type = type;
		this->defined = defined;
	}


	Symbol* next = nullptr;

	string getName() {
		return name;
	}
	string getSection() {
		return section;
	}
	int getOffset() {
		return offset;
	}
	string getType() {
		return type;
	}
	bool getDefined() {
		return defined;
	}

	Symbol& setName(string name) {
		this->name = name;
		return *this;
	}
	Symbol& setSection(string section) {
		this->section = section;
		return *this;
	}
	Symbol& setOffset(int offset) {
		this->offset = offset;
		return *this;
	}
	Symbol& setType(string type) {
		this->type = type;
		return *this;
	}
	Symbol& setDefined(bool defined) {
		this->defined = defined;
		return *this;
	}

	friend ostream& operator<<(ostream& out, const Symbol& s)
	{
		out << s.name << " " << s.section << " " << s.offset << " " << s.type;
		return out;
	}

	void addData(string s) {
		data += s;
	}

	void setData(string s) {
		data.assign(s);
	}

	string getData() {
		return data;
	}

	void addRelocationRecord(string offset, string type, string symbol) {
		relocationRecord += offset + " " + type + " " + symbol + "\n";
	}

	string getRelocationRecord() {
		return relocationRecord;
	}

	void addToIoC(string section) {
		map<string, int>::iterator i;
		for (i = IoC.begin(); i != IoC.end(); ++i) {
			if (i->first == section) {
				i->second = i->second + 1;
				if (i->second == 0) IoC.erase(section);
				return;
			}
		}
		IoC.insert(make_pair(section, 1));
	}

	void removeFromIoC(string section) {
		map<string, int>::iterator i;
		for (i = IoC.begin(); i != IoC.end(); ++i) {
			if (i->first == section) {
				if(section!="UND") i->second = i->second - 1;
				else i->second = i->second + 1;
				if (i->second == 0) IoC.erase(section);
				return;
			}
		}
		IoC.insert(make_pair(section, -1));
	}

	int getIoC() {
		if(IoC.size()!=0 && IoC.size()!=1) return IoC.size();
		else if (IoC.size() == 0) return 0;
		else {
			map<string, int>::iterator i;
			i = IoC.begin();
			return i->second;
		}
	}

	string getIoCSection() {
		if (IoC.size() == 0) {
			return "ABS";
		}
		else if (IoC.size() == 1) {
			map<string, int>::iterator i;
			i = IoC.begin();
			return i->first;
		}
		else {
			return "";
		}
	}


};


#endif