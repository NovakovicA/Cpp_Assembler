#ifndef SYMTABLE_H
#include <iostream>
#include <string>
#include "Symbol.h"

using namespace std;


class symTable {
	Symbol* first;
	Symbol* last;
	forwardRefs* ref;

public:
	symTable();
	int insert(Symbol s);
	int insert(Symbol *s); 
	void setGlobal(string s);
	~symTable();
	void show();
	friend ostream& operator<<(ostream& out, const symTable& s);
	friend ostream& operator<<(ostream& out, const symTable* s);
	void addData(string sym, string data);
	void addRelocationRecord(string sym, string offset, string type, string symbol);
	void checkUndefined();
	bool isDefined(string sym);
	bool isGlobal(string sym);
	int getOffset(string sym);
	string getSection(string sym);
	void addBackpatchingRecord(string sym, string section, int offset, int length, int type = 1,bool minus=false);
	Symbol* findSymbol(string sym);
	void backpatch();
	void equPatch();
};


#endif 
