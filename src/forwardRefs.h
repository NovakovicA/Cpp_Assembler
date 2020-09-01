#ifndef FORWARDREFS_H
#define FORWARDREFS_H
#include <iostream>
#include <string>


using namespace std;

class Elem {
public:
	string symbol;
	string section;
	int offset;
	int length;
	int type;
	bool minus;
	bool used=false;
	Elem* next = nullptr;
	Elem(string symbol, string section, int offset, int length, int type,bool minus) {
		this->symbol = symbol;
		this->offset = offset;
		this->section = section;
		this->length = length;
		this->type = type;
		this->minus = minus;
	}
};


class forwardRefs {

public:
	Elem* first;
	Elem* last;
	forwardRefs() {
		first = last = nullptr;
	}

	void add(string symbol, string section, int offset, int length, int type,bool minus) {
		if (first == nullptr) { first = last = new Elem(symbol, section, offset, length, type,minus); }
		else {
			last->next = new Elem(symbol, section, offset, length, type,minus);
			last = last->next;
		}
	}

	~forwardRefs() {
		Elem* old = nullptr;
		for (Elem* el = first; el != nullptr; el = el->next) {
			if (old != nullptr) { delete old; }
			old = el;
		}
		if (old != nullptr) { delete old; }
	}


};

#endif