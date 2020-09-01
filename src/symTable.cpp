#include <iostream>
#include <string>
#include "Symbol.h"
#include "Conversion.h"
#include "forwardRefs.h"
#include "symTable.h"


	 symTable::symTable() {
		 first = last = nullptr;
		 ref = new forwardRefs();
	}

	 int symTable::insert(Symbol s) {
		 if (first == nullptr) { first = last = new Symbol(s); return 0; }
		 else {
			 for (Symbol* el = first; el != nullptr; el = el->next) {
				 if (el->getName() == s.getName()) {
					if(el->getDefined() == true) return -1;
					else {
						el->setDefined(true);
						el->setSection(s.getSection());
						el->setOffset(s.getOffset());
						return 0;
					}
				 }
			 }
			 last->next = new Symbol(s);
			 last = last->next;
			 return 0;
		 }
	 }


	int symTable::insert(Symbol *s) {
		 if (first == nullptr) { first = last = new Symbol(*s); return 0; }
		 else {
			 for (Symbol* el = first; el != nullptr; el = el->next) {
				 if (el->getName() == s->getName()) {
					 if (el->getDefined() == true) return -1;
					 else {
						 el->setDefined(true);
						 el->setSection(s->getSection());
						 el->setOffset(s->getOffset());
						 return 0;
					 }
				 }
			 }
			 last->next = new Symbol(*s);
			 last = last->next;
			 return 0;
		 }
	 }

	void symTable::setGlobal(string s) {
		bool found = false;
		for (Symbol* el = first; el != nullptr; el = el->next) {
			if (el->getName() == s) {
				found = true;
				el->setType("global");
				break;
			}
		}
		if (!found) {
			insert(new Symbol(s, "", 0, "global", false));
		}
	}

	symTable::~symTable() {
		 Symbol* old = nullptr;
		 for (Symbol* el = first; el != nullptr; el = el->next) {
			 if (old != nullptr) { delete old; }
			 old = el;
		 }
		 if (old != nullptr) { delete old; }
		 delete ref;
	 }


	void symTable::show() {
		int index = 0;
		cout << "#--Symbol table--" << endl;
		cout << "#index name  section  offset type" << endl;
		for (Symbol* el = first; el != nullptr; el = el->next) {
			cout << index << "   " << (*el) << endl;
			index++;
		}
		cout << "#--End of symbol table--" << endl << endl;

		for (Symbol* el = first; el != nullptr; el = el->next) {
			if (el->getName() == el->getSection() && el->getRelocationRecord() != "") {
				cout << "# Relocation record - " << el->getName() << endl;
				cout << "# offset   type   symbol " << endl;
				cout << el->getRelocationRecord() << endl;
			}
		}

		for (Symbol* el = first; el != nullptr; el = el->next) {
			if (el->getName() == el->getSection() && el->getData() != "") {
				cout << "# " << el->getName() << endl;
				cout << el->getData() << endl;
			}
		}
	 }

	ostream& operator<<(ostream& out, const symTable& s)
	{
		int index = 0;
		out << "#--Symbol table--" << endl;
		out << "#index name  section  offset type" << endl;
		for (Symbol* el = s.first; el != nullptr; el = el->next) {
			out << index << "   " << (*el) << endl;
			index++;
		}
		out << "#--End of symbol table--" << endl << endl;
		for (Symbol* el = s.first; el != nullptr; el = el->next) {
			if (el->getName() == el->getSection() && el->getRelocationRecord() != "") {
				out << "# Relocation record - " << el->getName() << endl;
				out << "# offset   type   symbol "  << endl;
				out << el->getRelocationRecord() << endl;
			}
		}

		for (Symbol* el = s.first; el != nullptr; el = el->next) {
			if (el->getName() == el->getSection() && el->getData() != "") {
				out << "# " << el->getName() << endl;
				out << el->getData() << endl;
			}
		}
		return out;
	}
	

	ostream& operator<<(ostream& out, const symTable* s)
	{
		int index = 1;
		out << "--Symbol table--" << endl;
		out << "index    name  section  offset type" << endl;
		for (Symbol* el = s->first; el != nullptr; el = el->next) {
			out << index << "   " << (*el)  << endl;
			index++;
		}
		out << "--End of symbol table--" << endl << endl;
		for (Symbol* el = s->first; el != nullptr; el = el->next) {
			if (el->getName() == el->getSection() && el->getRelocationRecord() != "") {
				out << "# Relocation record - " << el->getName() << endl;
				out << "# offset   type   symbol " << endl;
				out << el->getRelocationRecord() << endl;
			}
		}

		for (Symbol* el = s->first; el != nullptr; el = el->next) {
			if (el->getName() == el->getSection() && el->getData() != "") {
				out << "# " << el->getName() << endl;
				out << el->getData() << endl;
			}
		}
		return out;
	}
	

	void symTable::addData(string sym,string data) {
		for (Symbol* el = first; el != nullptr; el = el->next) {
			if (el->getName() == sym) {
				el->addData(data);
			}
		}
	}

	void symTable::addRelocationRecord(string sym, string offset, string type, string symbol) {
		for (Symbol* el = first; el != nullptr; el = el->next) {
			if (el->getName() == sym) {
				el->addRelocationRecord(offset, type, symbol);
			}
		}
	}

	void symTable::checkUndefined() {
		for (Symbol* el = first; el != nullptr; el = el->next) {
			if (el->getDefined()==false) {
				cout << "Error: Used symbol undefined - " << el->getName()<< "." << endl;
				exit(-1);
			}
		}
	}

	bool symTable::isDefined(string sym) {
		for (Symbol* el = first; el != nullptr; el = el->next) {
			if (el->getName() == sym) {
				if(el->getDefined()==true) return true;
				else return false;
			}
		}
			insert(new Symbol(sym, "", 0, "local", false));
			return false;	
	}

	bool symTable::isGlobal(string sym) {
		for (Symbol* el = first; el != nullptr; el = el->next) {
			if (el->getName() == sym) {
				if (el->getType() == "global") return true;
				else return false;
			}
		}
		insert(new Symbol(sym, "", 0, "local", false));
		return false;
	}

	int symTable::getOffset(string sym) {
		for (Symbol* el = first; el != nullptr; el = el->next) {
			if (el->getName() == sym) {
				return el->getOffset();
			}
		}
		return -1;
	}

	string symTable::getSection(string sym) {
		for (Symbol* el = first; el != nullptr; el = el->next) {
			if (el->getName() == sym) {
				return el->getSection();
			}
		}
		return "";
	}

	void symTable::addBackpatchingRecord(string sym, string section, int offset, int length,int type,bool minus) {
				ref->add(sym, section, offset, length, type,minus);
	}


	Symbol* symTable::findSymbol(string sym) {
		for (Symbol* el = first; el != nullptr; el = el->next) {
			if (el->getName() == sym) return el;
		}
		return nullptr;
	}

	void symTable::backpatch() {
		for (Symbol* sym = first; sym != nullptr; sym = sym->next) {
			for (Elem* el = ref->first; el != nullptr; el = el->next) {
				if (sym->getName() == el->symbol && sym->getDefined()==true && el->length!=0 && el->used==false) {
					Symbol* patch = findSymbol(el->section);
					string data = patch->getData();
					string change = data.substr(el->offset*3, el->length * 3);
					change =  regex_replace(change, regex(" "), "");

					if (el->length == 1) {
						if (startsWith(change, "F")) change = "FFFFFF" + change;
					}
					else {
						string part2 = change.substr(0, 2);
						string part1 = change.substr(2, 2);
						change = part1 + part2;						
						if (startsWith(change, "F")) change = "FFFF" + change;
					}

					if (el->length == 1) {
						change = decToHexByte(hexToDec(change) + sym->getOffset());
					}
					else {
						change = decToHexWord(hexToDec(change) + sym->getOffset());
					}
					data.replace(el->offset * 3, el->length * 3, change);
					patch->setData(data);
					if (el->type == 1 && sym->getSection()!="ABS") {
						addRelocationRecord(patch->getName(), decToHex(el->offset), "R_16", sym->getSection());
					}
					else if (el->type == 2 && patch->getName() != sym->getSection() && sym->getSection() != "ABS") {
						addRelocationRecord(patch->getName(), decToHex(el->offset), "R_PC16", sym->getSection());
					}
					el->used = true;
				}
			}
		}
	}



	void symTable::equPatch() {
		int numOfPatched = 1; 
		while(numOfPatched!=0){
			numOfPatched = 0;
		for (Symbol* sym = first; sym != nullptr; sym = sym->next) {
			for (Elem* el = ref->first; el != nullptr; el = el->next) {
				if (sym->getName() == el->symbol && sym->getDefined() == true && el->type==0 && el->length==0 && el->offset==0 && el->used==false) {
					Symbol* patch = findSymbol(el->section);
					if (!el->minus) { patch->setOffset(patch->getOffset() + sym->getOffset()); if (sym->getSection() != "ABS") patch->addToIoC(sym->getSection()); }
					else { patch->setOffset(patch->getOffset() - sym->getOffset());  if (sym->getSection() != "ABS") patch->removeFromIoC(sym->getSection());}
					patch->numOfPatches--;
					el->used = true;
					if (patch->numOfPatches == 0) {
						if (patch->getIoC() != 0 && patch->getIoC() != 1) {
							cout << "Error: Symbol " << patch->getName() << " specified in equ directive has invalid definition (invalid index of classification)"  << "." << endl;
							exit(-1);
						}
						patch->setSection(patch->getIoCSection());
						patch->setDefined(true);
						numOfPatched++;
					}
				}
			}
		}
		}
	}