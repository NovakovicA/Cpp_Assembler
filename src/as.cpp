#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <vector>
#include <cmath> 
#include "Symbol.h"
#include "symTable.h"
#include "Conversion.h"

using namespace std;

int currentLine = 0;

string currentSection = "";
int currentSectionSize = 0;
symTable symtable;

bool validSection(string s) {
	if (s == ".bss" || s == ".data" || s == ".text" || s == ".rodata") {
		return true;
	}
	else if (startsWith(s, ".section ")) {
		return true;
	}
	else {
		return false;
	}
}

int validInstruction(string s) {
	if (startsWith(s, "halt") || startsWith(s, "iret") || startsWith(s, "ret")) return 1;
	else if (startsWith(s, "int ") || startsWith(s, "call ") || startsWith(s, "jmp ") || startsWith(s, "jeq ")
		|| startsWith(s, "jne ") || startsWith(s, "jgt ")) return 2;
	else if (startsWith(s, "push ") || startsWith(s, "pop ") || startsWith(s, "pushw ") || startsWith(s, "popw ") || startsWith(s, "pushb ") || startsWith(s, "popb ") || startsWith(s, "pushbh ") || startsWith(s, "pushbl ") || startsWith(s, "pushbh ") || startsWith(s, "popbl ")) return 3;
	else if (startsWith(s, "xchg") ||  startsWith(s, "mov") || startsWith(s, "add") || startsWith(s, "sub") || startsWith(s, "mul")
		|| startsWith(s, "div") || startsWith(s, "cmp") || startsWith(s, "not") || startsWith(s, "and")
		|| startsWith(s, "or") || startsWith(s, "xor") || startsWith(s, "test") || startsWith(s, "shl")
		|| startsWith(s, "shr")) return 4;
	else return 0;
}


void allocateInstruction1(string str) {
	str = regex_replace(str, regex(" "), "");
	if (str == "halt") {
		symtable.addData(currentSection, decToHexByte(0));
		currentSectionSize += 1;
	}
	else if (str == "iret") {
		symtable.addData(currentSection, decToHexByte(8));
		currentSectionSize += 1;
	}
	else if (str == "ret") {
		symtable.addData(currentSection, decToHexByte(16));
		currentSectionSize += 1;
	}
	else {
		cout << "Error: Invalid instruction on line " << currentLine << "." << endl;
		exit(-1);
	}
}

void allocateInstruction2(string str) {
	if (startsWith(str, "int ")) {
		str = regex_replace(str, regex("int "), "");
		symtable.addData(currentSection, decToHexByte(28));
		currentSectionSize += 1;
	}
	else if (startsWith(str, "call ")) {
		str = regex_replace(str, regex("call "), "");
		symtable.addData(currentSection, decToHexByte(36));
		currentSectionSize += 1;
	}
	else if (startsWith(str, "jmp ")) {
		str = regex_replace(str, regex("jmp "), "");
		symtable.addData(currentSection, decToHexByte(44));
		currentSectionSize += 1;
	}
	else if (startsWith(str, "jeq ")) {
		str = regex_replace(str, regex("jeq "), "");
		symtable.addData(currentSection, decToHexByte(52));
		currentSectionSize += 1;
	}
	else if (startsWith(str, "jne ")) {
		str = regex_replace(str, regex("jne "), "");
		symtable.addData(currentSection, decToHexByte(60));
		currentSectionSize += 1;
	}
	else if (startsWith(str, "jgt ")) {
		str = regex_replace(str, regex("jgt "), "");
		symtable.addData(currentSection, decToHexByte(68));
		currentSectionSize += 1;
	}
	else {
		cout << "Error: Invalid instruction on line " << currentLine << "." << endl;
		exit(-1);
	}

	if (regex_match(str, regex("(\\+|-)?[[:digit:]]+"))) {
		symtable.addData(currentSection, decToHexByte(0));
		currentSectionSize += 1;
		symtable.addData(currentSection, decToHexWord(stoi(str)));
		currentSectionSize += 2;
	}

	else if (regex_match(str, regex("0x[ABCDEFabcdef0-9]{1,4}"))) {
		str = regex_replace(str, regex("0x"), "");
		str = regex_replace(str, regex(" "), "");
		str = toUpper(str);
		while (str.length() < 4) str = "0" + str;
		string part2 = str.substr(0, 2);
		string part1 = str.substr(2, 2);
		str = part1 + " " + part2;
		symtable.addData(currentSection, decToHexByte(0));
		currentSectionSize += 1;
		symtable.addData(currentSection, str + " ");
		currentSectionSize += 2;
	}

	else if (regex_match(str, regex("[a-zA-Z0-9]+"))) {
		symtable.addData(currentSection, decToHexByte(0));
		currentSectionSize += 1;
		if (symtable.isGlobal(str)==true && symtable.getSection(str)=="UND") {
		if (symtable.getSection(str) != "ABS") symtable.addRelocationRecord(currentSection, decToHex(currentSectionSize), "R_16", str);
		symtable.addData(currentSection, decToHexWord(0));
		currentSectionSize += 2;
		}
		else if (symtable.isDefined(str)==true){
			symtable.addRelocationRecord(currentSection, decToHex(currentSectionSize), "R_16", symtable.getSection(str));
			symtable.addData(currentSection, decToHexWord(symtable.getOffset(str)));
			currentSectionSize += 2;
		}
		else {
			symtable.addBackpatchingRecord(str, currentSection, currentSectionSize, 2);
			symtable.addData(currentSection, decToHexWord(0));
			currentSectionSize += 2;
		}
	}

	else if (regex_match(str, regex("\\*%r[0-9]+"))) {
		str = regex_replace(str, regex("\\*%r"), "");
		if (stoi(str) > 15) {
			cout << "Error: Invalid instruction - registry out of scope on line " << currentLine << "." << endl;
			exit(-1);
		}
		
		symtable.addData(currentSection, decToHexByte((stoi(str)*2)+32));
		currentSectionSize += 1;
	}

	else if (regex_match(str, regex("\\*\\(%r[0-9]+\\)"))) {
		str = regex_replace(str, regex("\\*\\(%r"), "");
		if (stoi(str) > 15) {
			cout << "Error: Invalid instruction - registry out of scope on line " << currentLine << "." << endl;
			exit(-1);
		}
		symtable.addData(currentSection, decToHexByte((stoi(str) * 2) + 64));
		currentSectionSize += 1;
	}

	else if (regex_match(str, regex("\\*(\\+|-)?[[:digit:]]+\\(%r[0-9]+\\)"))) {
		vector<string> strs = split(str, "(");
		bool first = false;
		int number=0;
		for(string s:strs){
			if(!first){
				s = regex_replace(s, regex("\\*"), "");
				number = stoi(s);
				first = true;
			}
			else {
				s = regex_replace(s, regex("%r"), "");
				s = regex_replace(s, regex("\\)"), "");
				if (stoi(s) > 15) {
					cout << "Error: Invalid instruction - registry out of scope on line " << currentLine << "." << endl;
					exit(-1);
				}
				symtable.addData(currentSection, decToHexByte((stoi(s) * 2) + 96));
				currentSectionSize += 1;
				symtable.addData(currentSection, decToHexWord(number));
				currentSectionSize += 2;
			}
		}
	}

	else if (regex_match(str, regex("\\*0x[ABCDEFabcdef0-9]{1,4}\\(%r[0-9]+\\)"))) {
		vector<string> strs = split(str, "(");
		bool first = false;
		string num = "";
		for (string s : strs) {
			if (!first) {
				s = regex_replace(s, regex("\\*0x"), "");
				s = regex_replace(s, regex(" "), "");
				s = toUpper(s);
				while (s.length() < 4) s = "0" + s;
				string part2 = s.substr(0, 2);
				string part1 = s.substr(2, 2);
				s = part1 + " " + part2;
				num = s;
				first = true;
			}
			else {
				s = regex_replace(s, regex("%r"), "");
				s = regex_replace(s, regex("\\)"), "");
				if (stoi(s) > 15) {
					cout << "Error: Invalid instruction - registry out of scope on line " << currentLine << "." << endl;
					exit(-1);
				}
				symtable.addData(currentSection, decToHexByte((stoi(s) * 2) + 96));
				currentSectionSize += 1;
				symtable.addData(currentSection, num+" ");
				currentSectionSize += 2;
			}
		}
	}

	else if (regex_match(str, regex("\\*[a-zA-Z0-9]+\\(%r7\\)")) || regex_match(str, regex("\\*[a-zA-Z0-9]+\\(%pc\\)"))) {
		vector<string> strs = split(str, "(");
		for (string s : strs) {
			s = regex_replace(s, regex("\\*"), "");
			symtable.addData(currentSection, decToHexByte(14 + 96));
			currentSectionSize += 1;
			if (symtable.isGlobal(s) == true && symtable.getSection(s) == "UND") {
				symtable.addRelocationRecord(currentSection, decToHex(currentSectionSize), "R_PC16", s);
				symtable.addData(currentSection, decToHexWord(-2));
				currentSectionSize += 2;
			}
			else if (symtable.isDefined(s) == true) {
				if (symtable.getSection(s) != currentSection && (symtable.getSection(s) != "ABS")) {
					symtable.addRelocationRecord(currentSection, decToHex(currentSectionSize), "R_PC16", symtable.getSection(s));
				}
				symtable.addData(currentSection, decToHexWord(-2 + symtable.getOffset(s)));
				currentSectionSize += 2;
			}
			else {
				symtable.addBackpatchingRecord(s, currentSection, currentSectionSize, 2,2);
				symtable.addData(currentSection, decToHexWord(-2));
				currentSectionSize += 2;
			}

			break;
		}
	}

	else if (regex_match(str, regex("\\*[a-zA-Z0-9]+\\(%r[0-9]+\\)"))) {
		vector<string> strs = split(str, "(");
		bool first = false;
		string sym;
		for (string s : strs) {
			if (!first) {
				s = regex_replace(s, regex("\\*"), "");
				sym = s;
				first = true;
			}
			else {
				s = regex_replace(s, regex("%r"), "");
				s = regex_replace(s, regex("\\)"), "");
				if (stoi(s) > 15) {
					cout << "Error: Invalid instruction - registry out of scope on line " << currentLine << "." << endl;
					exit(-1);
				}
				symtable.addData(currentSection, decToHexByte((stoi(s) * 2) + 96));
				currentSectionSize += 1;
				if (symtable.isGlobal(sym) == true && symtable.getSection(sym) == "UND") {
					symtable.addRelocationRecord(currentSection, decToHex(currentSectionSize), "R_16", sym);
					symtable.addData(currentSection, decToHexWord(0));
					currentSectionSize += 2;
				}
				else if (symtable.isDefined(sym) == true) {
					if (symtable.getSection(sym) != "ABS") symtable.addRelocationRecord(currentSection, decToHex(currentSectionSize), "R_16", symtable.getSection(sym));
					symtable.addData(currentSection, decToHexWord(symtable.getOffset(sym)));
					currentSectionSize += 2;
				}
				else {
					symtable.addBackpatchingRecord(sym, currentSection, currentSectionSize, 2);
					symtable.addData(currentSection, decToHexWord(0));
					currentSectionSize += 2;
				}
			}
		}
	}


	else if (regex_match(str, regex("\\*(\\+|-)?[[:digit:]]+"))) {
		str = regex_replace(str, regex("\\*"), "");
		symtable.addData(currentSection, decToHexByte(128));
		currentSectionSize += 1;
		symtable.addData(currentSection, decToHexWord(stoi(str)));
		currentSectionSize += 2;
	}

	else if (regex_match(str, regex("\\*0x[ABCDEFabcdef0-9]{1,4}"))) {
		str = regex_replace(str, regex("\\*0x"), "");
		str = regex_replace(str, regex(" "), "");
		str = toUpper(str);
		while (str.length() < 4) str = "0" + str;
		string part2 = str.substr(0, 2);
		string part1 = str.substr(2, 2);
		str = part1 + " " + part2;
		symtable.addData(currentSection, decToHexByte(128));
		currentSectionSize += 1;
		symtable.addData(currentSection, str + " ");
		currentSectionSize += 2;
	}

	else if (regex_match(str, regex("\\*[a-zA-Z0-9]+"))) {
		str = regex_replace(str, regex("\\*"), "");
		symtable.addData(currentSection, decToHexByte(128));
		currentSectionSize += 1;
		if (symtable.isGlobal(str) == true && symtable.getSection(str) == "UND") {
			symtable.addRelocationRecord(currentSection, decToHex(currentSectionSize), "R_16", str);
			symtable.addData(currentSection, decToHexWord(0));
			currentSectionSize += 2;
		}
		else if (symtable.isDefined(str) == true) {
			symtable.addRelocationRecord(currentSection, decToHex(currentSectionSize), "R_16", symtable.getSection(str));
			symtable.addData(currentSection, decToHexWord(symtable.getOffset(str)));
			currentSectionSize += 2;
		}
		else {
			symtable.addBackpatchingRecord(str, currentSection, currentSectionSize, 2);
			symtable.addData(currentSection, decToHexWord(0));
			currentSectionSize += 2;
		}
	}


	else {
		cout << "Error: Invalid operand for instruction on line " << currentLine << "." << endl;
		exit(-1);
	}

}

void allocateInstruction3(string str) {
	int sizeOfOperand = 0;
	int pop = 0;
	int H = -1; //za H bitove u reg dir 1, u suprotnom 0

	if (startsWith(str, "push ") || (startsWith(str, "pushw "))) {
		str = regex_replace(str, regex("push "), "");
		str = regex_replace(str, regex("pushw "), "");
		symtable.addData(currentSection, decToHexByte(76));
		currentSectionSize += 1;
		sizeOfOperand = 2; 
	}

	else if (startsWith(str, "pushb ")) {
		str = regex_replace(str, regex("pushb "), "");
		symtable.addData(currentSection, decToHexByte(72));
		currentSectionSize += 1;
		sizeOfOperand = 1;
	}

	else if (startsWith(str, "pushbh ")) {
		str = regex_replace(str, regex("pushbh "), "");
		symtable.addData(currentSection, decToHexByte(72));
		currentSectionSize += 1;
		sizeOfOperand = 1; H = 1;
	}

	else if (startsWith(str, "pushbl ")) {
		str = regex_replace(str, regex("pushbl "), "");
		symtable.addData(currentSection, decToHexByte(72));
		currentSectionSize += 1;
		sizeOfOperand = 1; H = 0;
	}

	else if (startsWith(str, "pop ") || (startsWith(str, "popw "))) {
		str = regex_replace(str, regex("pop "), "");
		str = regex_replace(str, regex("popw "), "");
		symtable.addData(currentSection, decToHexByte(84));
		currentSectionSize += 1;
		pop = 1;
		sizeOfOperand = 2;
	}

	else if (startsWith(str, "popb ")) {
		str = regex_replace(str, regex("popb "), "");
		symtable.addData(currentSection, decToHexByte(80));
		currentSectionSize += 1;
		pop = 1;
		sizeOfOperand = 1;
	}

	else if (startsWith(str, "popbh ")) {
		str = regex_replace(str, regex("popbh "), "");
		symtable.addData(currentSection, decToHexByte(80));
		currentSectionSize += 1;
		pop = 1;
		sizeOfOperand = 1; H = 1;
	}

	else if (startsWith(str, "popbl ")) {
		str = regex_replace(str, regex("popbl "), "");
		symtable.addData(currentSection, decToHexByte(80));
		currentSectionSize += 1;
		pop = 1;
		sizeOfOperand = 1; H = 0;
	}

	else {
		cout << "Error: Invalid instruction on line " << currentLine << "." << endl;
		exit(-1);
	}


	if (regex_match(str, regex("\\$(\\+|-)?[[:digit:]]+"))) {
		if (H != -1) {
			cout << "Error: Invalid instruction on line " << currentLine << "." << endl;
			exit(-1);
		}
		if (pop == 1) {
			cout << "Error: Immediate value is not allowed as destination, line - " << currentLine << "." << endl;
			exit(-1);
		}
		str = regex_replace(str, regex("\\$"), "");
		symtable.addData(currentSection, decToHexByte(0));
		currentSectionSize += 1;
		if(sizeOfOperand==2){
		symtable.addData(currentSection, decToHexWord(stoi(str)));
		currentSectionSize += 2;
		}
		else {
			symtable.addData(currentSection, decToHexByte(stoi(str)));
			currentSectionSize += 1;
		}
	}

	else if (regex_match(str, regex("\\$0x[ABCDEFabcdef0-9]{1,4}"))) {
		if (H != -1) {
			cout << "Error: Invalid instruction on line " << currentLine << "." << endl;
			exit(-1);
		}
		if (pop == 1) {
			cout << "Error: Immediate value is not allowed as destination, line - " << currentLine << "." << endl;
			exit(-1);
		}
		str = regex_replace(str, regex("\\$0x"), "");
		str = regex_replace(str, regex(" "), "");
		str = toUpper(str);
		symtable.addData(currentSection, decToHexByte(0));
		currentSectionSize += 1;
		if (sizeOfOperand == 2) {
			while (str.length() < 4) str = "0" + str;
			string part2 = str.substr(0, 2);
			string part1 = str.substr(2, 2);
			str = part1 + " " + part2;
			symtable.addData(currentSection, str + " ");
			currentSectionSize += 2;
		}
		else {
			if (str.length() > 2) str.resize(2);
			symtable.addData(currentSection, str + " " );
			currentSectionSize += 1;
		}
	}

	else if (regex_match(str, regex("\\$[a-zA-Z0-9]+"))) {
		if (H != -1) {
			cout << "Error: Invalid instruction on line " << currentLine << "." << endl;
			exit(-1);
		}
		if (pop == 1) {
			cout << "Error: Immediate value is not allowed as destination, line - " << currentLine << "." << endl;
			exit(-1);
		}
		str = regex_replace(str, regex("\\$"), "");
		symtable.addData(currentSection, decToHexByte(0));
		currentSectionSize += 1;
		if(sizeOfOperand==2){
		if (symtable.isGlobal(str) == true && symtable.getSection(str) == "UND") {
			symtable.addRelocationRecord(currentSection, decToHex(currentSectionSize), "R_16", str);
			symtable.addData(currentSection, decToHexWord(0));
			currentSectionSize += 2;
		}
		else if (symtable.isDefined(str) == true) {
			if (symtable.getSection(str) != "ABS") symtable.addRelocationRecord(currentSection, decToHex(currentSectionSize), "R_16", symtable.getSection(str));
			symtable.addData(currentSection, decToHexWord(symtable.getOffset(str)));
			currentSectionSize += 2;
		}
		else {
			symtable.addBackpatchingRecord(str, currentSection, currentSectionSize, 2);
			symtable.addData(currentSection, decToHexWord(0));
			currentSectionSize += 2;
		}
		}
		else {
			if (symtable.isGlobal(str) == true && symtable.getSection(str) == "UND") {
				symtable.addRelocationRecord(currentSection, decToHexByte(currentSectionSize), "R_16", str);
				symtable.addData(currentSection, decToHexByte(0));
				currentSectionSize += 1;
			}
			else if (symtable.isDefined(str) == true) {
				if (symtable.getSection(str) != "ABS") symtable.addRelocationRecord(currentSection, decToHex(currentSectionSize), "R_16", symtable.getSection(str));
				symtable.addData(currentSection, decToHexByte(symtable.getOffset(str)));
				currentSectionSize += 1;
			}
			else {
				symtable.addBackpatchingRecord(str, currentSection, currentSectionSize, 1);
				symtable.addData(currentSection, decToHexByte(0));
				currentSectionSize += 1;
			}
		}
	}

	else if (regex_match(str, regex("\\%r[0-9]+"))) {
		str = regex_replace(str, regex("\\%r"), "");
		if (stoi(str) > 15) {
			cout << "Error: Invalid instruction - registry out of scope on line " << currentLine << "." << endl;
			exit(-1);
		}
		if (H == -1 && sizeOfOperand== 1) {
			cout << "Error: Invalid instruction - specify in what byte should the operand be placed - line " << currentLine << "." << endl;
			exit(-1);
		}
		else if (sizeOfOperand == 2) {
			symtable.addData(currentSection, decToHexByte((stoi(str) * 2) + 32));
			currentSectionSize += 1;
		}
		else {
			symtable.addData(currentSection, decToHexByte((stoi(str) * 2) + 32+H));
			currentSectionSize += 1;
		}
	}


	else if (regex_match(str, regex("\\(\\%r[0-9]+\\)"))) {
		if (H != -1) {
			cout << "Error: Invalid instruction on line " << currentLine << "." << endl;
			exit(-1);
		}

		str = regex_replace(str, regex("\\(\\%r"), "");
		str = regex_replace(str, regex("\\)"), "");
		if (stoi(str) > 15) {
			cout << "Error: Invalid instruction - registry out of scope on line " << currentLine << "." << endl;
			exit(-1);
		}
		symtable.addData(currentSection, decToHexByte((stoi(str) * 2) + 64));
		currentSectionSize += 1;
	}

	else if (regex_match(str, regex("(\\+|-)?[[:digit:]]+\\(%r[0-9]+\\)"))) {
		if (H != -1) {
			cout << "Error: Invalid instruction on line " << currentLine << "." << endl;
			exit(-1);
		}
		vector<string> strs = split(str, "(");
		bool first = false;
		int number = 0;
		for (string s : strs) {
			if (!first) {
				number = stoi(s);
				first = true;
			}
			else {
				s = regex_replace(s, regex("%r"), "");
				s = regex_replace(s, regex("\\)"), "");
				if (stoi(s) > 15) {
					cout << "Error: Invalid instruction - registry out of scope on line " << currentLine << "." << endl;
					exit(-1);
				}
				symtable.addData(currentSection, decToHexByte((stoi(s) * 2) + 96));
				currentSectionSize += 1;
				symtable.addData(currentSection, decToHexWord(number));
				currentSectionSize += 2;
			}
		}
	}

	else if (regex_match(str, regex("0x[ABCDEFabcdef0-9]{1,4}\\(%r[0-9]+\\)"))) {
		if (H != -1) {
			cout << "Error: Invalid instruction on line " << currentLine << "." << endl;
			exit(-1);
		}
		vector<string> strs = split(str, "(");
		bool first = false;
		string number = "";
		for (string s : strs) {
			if (!first) {
				s = regex_replace(s, regex("0x"), "");
				s = regex_replace(s, regex(" "), "");
				s = toUpper(s);
				while (s.length() < 4) s = "0" + s;
				string part2 = s.substr(0, 2);
				string part1 = s.substr(2, 2);
				s = part1 + " " + part2;
				number = s;
				first = true;
			}
			else {
				s = regex_replace(s, regex("%r"), "");
				s = regex_replace(s, regex("\\)"), "");
				if (stoi(s) > 15) {
					cout << "Error: Invalid instruction - registry out of scope on line " << currentLine << "." << endl;
					exit(-1);
				}
				symtable.addData(currentSection, decToHexByte((stoi(s) * 2) + 96));
				currentSectionSize += 1;
				symtable.addData(currentSection, number + " ");
				currentSectionSize += 2;
			}
		}
	}


	else if (regex_match(str, regex("[a-zA-Z0-9]+\\(%r7\\)")) || regex_match(str, regex("[a-zA-Z0-9]+\\(%pc\\)"))) {
		if (H != -1) {
			cout << "Error: Invalid instruction on line " << currentLine << "." << endl;
			exit(-1);
		}
		vector<string> strs = split(str, "(");
		for (string s : strs) {
			symtable.addData(currentSection, decToHexByte(14 + 96));
			currentSectionSize += 1;
			if (symtable.isGlobal(s) == true && symtable.getSection(s) == "UND") {
				symtable.addRelocationRecord(currentSection, decToHex(currentSectionSize), "R_PC16", s);
				symtable.addData(currentSection, decToHexWord(-2));
				currentSectionSize += 2;
			}
			else if (symtable.isDefined(s) == true) {
				if (symtable.getSection(s) != currentSection && (symtable.getSection(s) != "ABS")) {
					symtable.addRelocationRecord(currentSection, decToHex(currentSectionSize), "R_PC16", symtable.getSection(s));
				}
				symtable.addData(currentSection, decToHexWord(-2 + symtable.getOffset(s)));
				currentSectionSize += 2;
			}
			else {
				symtable.addBackpatchingRecord(s, currentSection, currentSectionSize, 2,2);
				symtable.addData(currentSection, decToHexWord(-2));
				currentSectionSize += 2;
			}
			break;
		}
	}

	else if (regex_match(str, regex("[a-zA-Z0-9]+\\(%r[0-9]+\\)"))) {
		if (H != -1) {
			cout << "Error: Invalid instruction on line " << currentLine << "." << endl;
			exit(-1);
		}
		vector<string> strs = split(str, "(");
		bool first = false;
		string sym;
		for (string s : strs) {
			if (!first) {
				sym = s;
				first = true;
			}
			else {
				s = regex_replace(s, regex("%r"), "");
				s = regex_replace(s, regex("\\)"), "");
				if (stoi(s) > 15) {
					cout << "Error: Invalid instruction - registry out of scope on line " << currentLine << "." << endl;
					exit(-1);
				}
				symtable.addData(currentSection, decToHexByte((stoi(s) * 2) + 96));
				currentSectionSize += 1;
				if (symtable.isGlobal(sym) == true && symtable.getSection(sym) == "UND") {
					symtable.addRelocationRecord(currentSection, decToHex(currentSectionSize), "R_16", sym);
					symtable.addData(currentSection, decToHexWord(0));
					currentSectionSize += 2;
				}
				else if (symtable.isDefined(sym) == true) {
					if (symtable.getSection(sym) != "ABS") symtable.addRelocationRecord(currentSection, decToHex(currentSectionSize), "R_16", symtable.getSection(sym));
					symtable.addData(currentSection, decToHexWord(symtable.getOffset(sym)));
					currentSectionSize += 2;
				}
				else {
					symtable.addBackpatchingRecord(sym, currentSection, currentSectionSize, 2);
					symtable.addData(currentSection, decToHexWord(0));
					currentSectionSize += 2;
				}
			}
		}
	}

	else if (regex_match(str, regex("(\\+|-)?[[:digit:]]+"))) {
		symtable.addData(currentSection, decToHexByte(128));
		currentSectionSize += 1;
		symtable.addData(currentSection, decToHexWord(stoi(str)));
		currentSectionSize += 2;
	}

	else if (regex_match(str, regex("0x[ABCDEFabcdef0-9]{1,4}"))) {
		str = regex_replace(str, regex("0x"), "");
		str = regex_replace(str, regex(" "), "");
		str = toUpper(str);
		while (str.length() < 4) str = "0" + str;
		string part2 = str.substr(0, 2);
		string part1 = str.substr(2, 2);
		str = part1 + " " + part2;
		symtable.addData(currentSection, decToHexByte(128));
		currentSectionSize += 1;
		symtable.addData(currentSection, str + " ");
		currentSectionSize += 2;
	}

	else if (regex_match(str, regex("[a-zA-Z0-9]+"))) {
		symtable.addData(currentSection, decToHexByte(128));
		currentSectionSize += 1;
		if (symtable.isGlobal(str) == true && symtable.getSection(str) == "UND") {
			symtable.addRelocationRecord(currentSection, decToHex(currentSectionSize), "R_16", str);
			symtable.addData(currentSection, decToHexWord(0));
			currentSectionSize += 2;
		}
		else if (symtable.isDefined(str) == true) {
			if (symtable.getSection(str) != "ABS") symtable.addRelocationRecord(currentSection, decToHex(currentSectionSize), "R_16", symtable.getSection(str));
			symtable.addData(currentSection, decToHexWord(symtable.getOffset(str)));
			currentSectionSize += 2;
		}
		else {
			symtable.addBackpatchingRecord(str, currentSection, currentSectionSize, 2);
			symtable.addData(currentSection, decToHexWord(0));
			currentSectionSize += 2;
		}
	}

	
	else {
		cout << "Error: Invalid operand for instruction on line " << currentLine << "." << endl;
		exit(-1);
	}

}

void allocateInstruction4(string str) {
	int sizeOfOperand = 0;
	int H = -1; //za H bitove u reg dir 1, u suprotnom 0
	vector<string> ins1 = { "xchg", "mov", "add", "sub", "mul", "div", "cmp", "not", "and", "or", "xor", "test", "shl", "shr" };
	vector<string> ins2 = { "xchgw", "movw", "addw", "subw", "mulw", "divw", "cmpw", "notw", "andw", "orw", "xorw", "testw", "shlw", "shrw" };
	vector<string> ins3 = { "xchgb", "movb", "addb", "subb", "mulb", "divb", "cmpb", "notb", "andb", "orb", "xorb", "testb", "shlb", "shrb" };
	vector<string> ins4 = { "xchgbh", "movbh", "addbh", "subbh", "mulbh", "divbh", "cmpbh", "notbh", "andbh", "orbh", "xorbh", "testbh", "shlbh", "shrbh" };
	vector<string> ins5 = { "xchgbl", "movbl", "addbl", "subbl", "mulbl", "divbl", "cmpbl", "notbl", "andbl", "orbl", "xorbl", "testbl", "shlbl", "shrbl" };
	vector<int> insc = { 11,12,13,14,15,16,17,18,19,20,21,22,23,24 };
	vector<string> strs = split(str, " ");
	int ind = -1; string rep;
	for (string s : strs) {
		rep = s;
		if ((ind = contains(ins1, s)) != -1) { sizeOfOperand = 2; break; }
		if ((ind = contains(ins2, s)) != -1) { sizeOfOperand = 2; break; }
		if ((ind = contains(ins3, s)) != -1) { sizeOfOperand = 1; break; }
		if ((ind = contains(ins4, s)) != -1) { sizeOfOperand = 1; H = 1; break; }
		if ((ind = contains(ins5, s)) != -1) { sizeOfOperand = 1; H = 0; break; }
		cout << "Error: Invalid instruction on line " << currentLine << "." << endl;
		exit(-1);
	}
	if (sizeOfOperand == 1) symtable.addData(currentSection, decToHexByte(insc[ind] * 8));
	else symtable.addData(currentSection, decToHexByte((insc[ind] * 8) + 4));
	currentSectionSize += 1;
	str = regex_replace(str, regex(rep), "");
	str = regex_replace(str, regex(" "), "");
	strs = split(str, ",");
	if (strs.size() != 2) {
		cout << "Error: Invalid operand for instruction on line " << currentLine << "." << endl;
		exit(-1);
	}

	bool dst = false;
	if (startsWith(rep, "xchg")) dst = true;

	int disp = -5;
	bool second = false;
	bool reg = false;
	for (string op : strs) {
		if (!second) second = true;
		else {
			if ((regex_match(op, regex("\\%r[0-9]+"))) || (regex_match(op, regex("\\(\\%r[0-9]+\\)")))) reg = true;
		}
	}
	if (reg == true) disp = -3;

	for (string op : strs) {
		if (regex_match(op, regex("\\$(\\+|-)?[[:digit:]]+"))) {
			if (dst) {
				cout << "Error: Immediate value is not allowed as destination, line - " << currentLine << "." << endl;
				exit(-1);
			}
			op = regex_replace(op, regex("\\$"), "");
			symtable.addData(currentSection, decToHexByte(0));
			currentSectionSize += 1;
			if (sizeOfOperand == 2) {
				symtable.addData(currentSection, decToHexWord(stoi(op)));
				currentSectionSize += 2;
			}
			else {
				symtable.addData(currentSection, decToHexByte(stoi(op)));
				currentSectionSize += 1;
			}
		}

		else if (regex_match(op, regex("\\$0x[ABCDEFabcdef0-9]{1,4}"))) {
			if (dst) {
				cout << "Error: Immediate value is not allowed as destination, line - " << currentLine << "." << endl;
				exit(-1);
			}
			op = regex_replace(op, regex("\\$0x"), "");
			op = toUpper(op);
			symtable.addData(currentSection, decToHexByte(0));
			currentSectionSize += 1;
			if (sizeOfOperand == 2) {
				while (op.length() < 4) op = "0" + op;
				string part2 = op.substr(0, 2);
				string part1 = op.substr(2, 2);
				op = part1 + " " + part2;
				symtable.addData(currentSection, op+" ");
				currentSectionSize += 2;
			}
			else {
				if (op.length() > 2) op.resize(2);
				symtable.addData(currentSection,op+ " ");
				currentSectionSize += 1;
			}
		}

		else if (regex_match(op, regex("\\$[a-zA-Z0-9]+"))) {
			if (dst) {
				cout << "Error: Immediate value is not allowed as destination, line - " << currentLine << "." << endl;
				exit(-1);
			}
			op = regex_replace(op, regex("\\$"), "");
			symtable.addData(currentSection, decToHexByte(0));
			currentSectionSize += 1;
			if(sizeOfOperand==2){
			if (symtable.isGlobal(op) == true && symtable.getSection(op) == "UND") {
				symtable.addRelocationRecord(currentSection, decToHex(currentSectionSize), "R_16", op);
				symtable.addData(currentSection, decToHexWord(0));
				currentSectionSize += 2;
			}
			else if (symtable.isDefined(op) == true) {
				if (symtable.getSection(op) != "ABS") symtable.addRelocationRecord(currentSection, decToHex(currentSectionSize), "R_16", symtable.getSection(op));
				symtable.addData(currentSection, decToHexWord(symtable.getOffset(op)));
				currentSectionSize += 2;
			}
			else {
				symtable.addBackpatchingRecord(op, currentSection, currentSectionSize, 2);
				symtable.addData(currentSection, decToHexWord(0));
				currentSectionSize += 2;
			}
			}
			else{
				if (symtable.isGlobal(op) == true && symtable.getSection(op) == "UND") {
					symtable.addRelocationRecord(currentSection, decToHex(currentSectionSize), "R_16", op);
					symtable.addData(currentSection, decToHexByte(0));
					currentSectionSize += 1;
				}
				else if (symtable.isDefined(op) == true) {
					if (symtable.getSection(op) != "ABS") symtable.addRelocationRecord(currentSection, decToHex(currentSectionSize), "R_16", symtable.getSection(op));
					symtable.addData(currentSection, decToHexByte(symtable.getOffset(op)));
					currentSectionSize += 1;
				}
				else {
					symtable.addBackpatchingRecord(op, currentSection, currentSectionSize, 1);
					symtable.addData(currentSection, decToHexByte(0));
					currentSectionSize += 1;
				}
			}
		}

		else if (regex_match(op, regex("\\%r[0-9]+"))) {
			op = regex_replace(op, regex("\\%r"), "");
			if (stoi(op) > 15) {
				cout << "Error: Invalid instruction - registry out of scope on line " << currentLine << "." << endl;
				exit(-1);
			}
			if (H == -1 && sizeOfOperand == 1) {
				cout << "Error: Invalid instruction - specify in what byte should the operand be placed - line " << currentLine << "." << endl;
				exit(-1);
			}
			else if (sizeOfOperand == 2) {
				symtable.addData(currentSection, decToHexByte((stoi(op) * 2) + 32));
				currentSectionSize += 1;
			}
			else {
				symtable.addData(currentSection, decToHexByte((stoi(op) * 2) + 32 + H));
				currentSectionSize += 1;
			}
		}


		else if (regex_match(op, regex("\\(\\%r[0-9]+\\)"))) {
			op = regex_replace(op, regex("\\(\\%r"), "");
			op = regex_replace(op, regex("\\)"), "");
			if (stoi(op) > 15) {
				cout << "Error: Invalid instruction - registry out of scope on line " << currentLine << "." << endl;
				exit(-1);
			}
			symtable.addData(currentSection, decToHexByte((stoi(op) * 2) + 64));
			currentSectionSize += 1;
		}

		else if (regex_match(op, regex("(\\+|-)?[[:digit:]]+\\(%r[0-9]+\\)"))) {
			if (H != -1) {
				cout << "Error: Invalid instruction on line " << currentLine << "." << endl;
				exit(-1);
			}
			vector<string> ops = split(op, "(");
			bool first = false;
			int number = 0;
			for (string s : ops) {
				if (!first) {
					number = stoi(s);
					first = true;
				}
				else {
					s = regex_replace(s, regex("%r"), "");
					s = regex_replace(s, regex("\\)"), "");
					if (stoi(s) > 15) {
						cout << "Error: Invalid instruction - registry out of scope on line " << currentLine << "." << endl;
						exit(-1);
					}
					symtable.addData(currentSection, decToHexByte((stoi(s) * 2) + 96));
					currentSectionSize += 1;
					symtable.addData(currentSection, decToHexWord(number));
					currentSectionSize += 2;
				}
			}
		}

		else if (regex_match(op, regex("0x[ABCDEFabcdef0-9]{1,4}\\(%r[0-9]+\\)"))) {
			if (H != -1) {
				cout << "Error: Invalid instruction on line " << currentLine << "." << endl;
				exit(-1);
			}
			vector<string> ops = split(op, "(");
			bool first = false;
			string number = "";
			for (string s : ops) {
				if (!first) {
					s = regex_replace(s, regex("0x"), "");
					s = toUpper(s);
					while (s.length() < 4) s = "0" + s;
					string part2 = s.substr(0, 2);
					string part1 = s.substr(2, 2);
					s = part1 + " " + part2;
					number = s;
					first = true;
				}
				else {
					s = regex_replace(s, regex("%r"), "");
					s = regex_replace(s, regex("\\)"), "");
					if (stoi(s) > 15) {
						cout << "Error: Invalid instruction - registry out of scope on line " << currentLine << "." << endl;
						exit(-1);
					}
					symtable.addData(currentSection, decToHexByte((stoi(s) * 2) + 96));
					currentSectionSize += 1;
					symtable.addData(currentSection, number + " ");
					currentSectionSize += 2;
				}
			}
		}


		else if (regex_match(op, regex("[a-zA-Z0-9]+\\(%r7\\)")) || regex_match(op, regex("[a-zA-Z0-9]+\\(%pc\\)"))) {
			vector<string> ops = split(op, "(");
			for (string s : ops) {
				symtable.addData(currentSection, decToHexByte(14 + 96));
				currentSectionSize += 1;
				if (symtable.isGlobal(s) == true && symtable.getSection(s) == "UND") {
					symtable.addRelocationRecord(currentSection, decToHex(currentSectionSize), "R_PC16", s);
					symtable.addData(currentSection, decToHexWord(disp));
					currentSectionSize += 2;
				}
				else if (symtable.isDefined(s) == true) {
					if (symtable.getSection(s) != currentSection && (symtable.getSection(s) != "ABS")) {
						symtable.addRelocationRecord(currentSection, decToHex(currentSectionSize), "R_PC16", symtable.getSection(s));
					}
					symtable.addData(currentSection, decToHexWord(disp + symtable.getOffset(s)));
					currentSectionSize += 2;
				}
				else {
					symtable.addBackpatchingRecord(s, currentSection, currentSectionSize, 2,2);
					symtable.addData(currentSection, decToHexWord(disp));
					currentSectionSize += 2;
				}
				break;
			}
		}

		else if (regex_match(op, regex("[a-zA-Z0-9]+\\(%r[0-9]+\\)"))) {
			vector<string> ops = split(op, "(");
			bool first = false;
			string sym;
			for (string s : ops) {
				if (!first) {
					sym = s;
					first = true;
				}
				else {
					s = regex_replace(s, regex("%r"), "");
					s = regex_replace(s, regex("\\)"), "");
					if (stoi(s) > 15) {
						cout << "Error: Invalid instruction - registry out of scope on line " << currentLine << "." << endl;
						exit(-1);
					}
					symtable.addData(currentSection, decToHexByte((stoi(s) * 2) + 96));
					currentSectionSize += 1;
					if (symtable.isGlobal(sym) == true && symtable.getSection(sym) == "UND") {
						symtable.addRelocationRecord(currentSection, decToHex(currentSectionSize), "R_16", sym);
						symtable.addData(currentSection, decToHexWord(0));
						currentSectionSize += 2;
					}
					else if (symtable.isDefined(sym) == true) {
						if (symtable.getSection(sym) != "ABS")	symtable.addRelocationRecord(currentSection, decToHex(currentSectionSize), "R_16", symtable.getSection(sym));
						symtable.addData(currentSection, decToHexWord(symtable.getOffset(sym)));
						currentSectionSize += 2;
					}
					else {
						symtable.addBackpatchingRecord(sym, currentSection, currentSectionSize, 2);
						symtable.addData(currentSection, decToHexWord(0));
						currentSectionSize += 2;
					}
				}
			}
		}

		else if (regex_match(op, regex("(\\+|-)?[[:digit:]]+"))) {
			symtable.addData(currentSection, decToHexByte(128));
			currentSectionSize += 1;
			symtable.addData(currentSection, decToHexWord(stoi(op)));
			currentSectionSize += 2;
		}

		else if (regex_match(op, regex("0x[ABCDEFabcdef0-9]{1,4}"))) {
			symtable.addData(currentSection, decToHexByte(128));
			currentSectionSize += 1;
			op = regex_replace(op, regex("0x"), "");
			op = toUpper(op);
			while (op.length() < 4) op = "0" + op;
			string part2 = op.substr(0, 2);
			string part1 = op.substr(2, 2);
			op = part1 + " " + part2;
			symtable.addData(currentSection, op+" ");
			currentSectionSize += 2;
		}

		else if (regex_match(op, regex("[a-zA-Z0-9]+"))) {
			symtable.addData(currentSection, decToHexByte(128));
			currentSectionSize += 1;
			if (symtable.isGlobal(op) == true && symtable.getSection(op) == "UND") {
				symtable.addRelocationRecord(currentSection, decToHex(currentSectionSize), "R_16", op);
				symtable.addData(currentSection, decToHexWord(0));
				currentSectionSize += 2;
			}
			else if (symtable.isDefined(op) == true) {
				if (symtable.getSection(op) != "ABS") symtable.addRelocationRecord(currentSection, decToHex(currentSectionSize), "R_16", symtable.getSection(op));
				symtable.addData(currentSection, decToHexWord(symtable.getOffset(op)));
				currentSectionSize += 2;
			}
			else {
				symtable.addBackpatchingRecord(op, currentSection, currentSectionSize, 2);
				symtable.addData(currentSection, decToHexWord(0));
				currentSectionSize += 2;
			}
		}
		else {
			cout << "Error: Invalid operand for instruction on line " << currentLine << "." << endl;
			exit(-1);
		}
		if(!startsWith(rep,"cmp") && !startsWith(rep,"test")) dst = true;
		disp = -2;
	}
}


void allocateMemory(string str) {
	if (currentSection == "") {
		cout << "Error: Data must be allocated within a section, line - " << currentLine << "." << endl;
		exit(-1);
	}

	if (startsWith(str, ".byte ")) {
		str = regex_replace(str, regex(".byte "), "");

		str = regex_replace(str, regex(" "), "");
		vector<string> strs = split(str, ",");

		for (string s : strs) {
			regex integer("(\\+|-)?[[:digit:]]+");
			regex hex("0x[ABCDEFabcdef0-9]{1,2}");

			if (regex_match(s, integer)) {
				int num = stoi(s);
				symtable.addData(currentSection, decToHexByte(num));
				currentSectionSize += 1;
			}

			else if (regex_match(s, hex)) {
				s = regex_replace(s, regex("0x"), "");
				s = regex_replace(s, regex(" "), "");
				s = toUpper(s);
				symtable.addData(currentSection, s + " ");
				currentSectionSize += 1;
			}

			else if (regex_match(s, regex("[a-zA-Z0-9]+"))) {
				if (symtable.isGlobal(s) == true && symtable.getSection(s) == "UND") {
					symtable.addRelocationRecord(currentSection, decToHex(currentSectionSize), "R_16", s);
					symtable.addData(currentSection, decToHexByte(0));
					currentSectionSize += 1;
				}
				else if (symtable.isDefined(s) == true) {
					if(symtable.getSection(s)!="ABS") symtable.addRelocationRecord(currentSection, decToHex(currentSectionSize), "R_16", symtable.getSection(s));
					symtable.addData(currentSection, decToHexByte(symtable.getOffset(s)));
					currentSectionSize += 1;
				}
				else {
					symtable.addBackpatchingRecord(s, currentSection, currentSectionSize, 1);
					symtable.addData(currentSection, decToHexByte(0));
					currentSectionSize += 1;
				}
			}

			else{
				cout << "Error: Operand specified within .byte directive is invalid on line " << currentLine << "." << endl;
			}
		}
	}

	else if (startsWith(str, ".word ")) {
		str = regex_replace(str, regex(".word "), "");

		str = regex_replace(str, regex(" "), "");
		vector<string> strs = split(str, ",");

		for (string s : strs) {
			regex integer("(\\+|-)?[[:digit:]]+");
			regex hex("0x[ABCDEFabcdef0-9]{1,4}");
			if (regex_match(s, integer)) {
				int num = stoi(s);
				symtable.addData(currentSection, decToHexWord(num));
				currentSectionSize += 2;
			}

			else if (regex_match(s, hex)) {
				s = regex_replace(s, regex("0x"), "");
				s = regex_replace(s, regex(" "), "");
				s = toUpper(s);
				while (s.length() < 4) s = "0" + s;
				string part2 = s.substr(0, 2);
				string part1 = s.substr(2, 2);
				s = part1 + " " + part2;
				symtable.addData(currentSection, s + " ");
				currentSectionSize += 2;
			}


			else if (regex_match(s, regex("[a-zA-Z0-9]+"))) {
				if (symtable.isGlobal(s) == true && symtable.getSection(s) == "UND") {
					symtable.addRelocationRecord(currentSection, decToHex(currentSectionSize), "R_16", s);
					symtable.addData(currentSection, decToHexWord(0));
					currentSectionSize += 2;
				}
				else if (symtable.isDefined(s) == true) {
					if (symtable.getSection(s) != "ABS") symtable.addRelocationRecord(currentSection, decToHex(currentSectionSize), "R_16", symtable.getSection(s));
					symtable.addData(currentSection, decToHexWord(symtable.getOffset(s)));
					currentSectionSize += 2;
				}
				else {
					symtable.addBackpatchingRecord(s, currentSection, currentSectionSize, 2);
					symtable.addData(currentSection, decToHexWord(0));
					currentSectionSize += 2;
				}
			}
			else {
				cout << "Error: Operand specified within .word directive is invalid on line " << currentLine << "." << endl;
				exit(-1);
			}
		}
	}

	else  if (startsWith(str, ".skip ")) {
		str = regex_replace(str, regex(".skip "), "");
		regex integer("(\\+|-)?[[:digit:]]+");
		if (!regex_match(str, integer)) {
			cout << "Error: Number specified within .skip directive is invalid on line " << currentLine << "." << endl;
			exit(-1);
		}
		int num = stoi(str);
		currentSectionSize += num;
		for (int i = 0; i<num; i++)
			symtable.addData(currentSection, "00 ");
	}
}




void process(const string& input, const string& output) {
	ifstream inputFile;
	ofstream outputFile;

	string str;
	inputFile.open(input);
	outputFile.open(output);

	if(inputFile.is_open() && outputFile.is_open()){
		symtable.insert(new Symbol("   ", "UND", 0, "local",true));
	while (getline(inputFile, str)) {

		str = toLower(str);
		str = regex_replace(str, regex("\r"), "");
		currentLine++;
	
		if (startsWith(str, ".")) {
			if (str == ".end") {
				break;
			}

			else if (validSection(str)){
			//new section
			if (startsWith(str, ".section ")) {
				str=regex_replace(str, regex("section "), "");
			}

			currentSection = str;
			currentSectionSize = 0;
			if (symtable.insert(new Symbol(str, str, 0, "local",true)) != 0) {
				cout << "Error: Redeclaration of section " << str << " on line " << currentLine << "." << endl;
				exit(-1);
			}
			}


			else if (startsWith(str, ".byte ") || startsWith(str, ".word ") || startsWith(str, ".skip ")) {
				allocateMemory(str);
				}

			else if (startsWith(str, ".extern ")) {
				if (currentSection == "") {
					str = regex_replace(str, regex(".extern "), "");
					str = regex_replace(str, regex(" "), "");
					vector<string> strs = split(str, ",");
					for (string s : strs) {
						symtable.insert(new Symbol(s, "UND", 0, "global", true));
					}
				}
				else {
					cout << "Error: Extern directive must not be within a section - line " << currentLine << "." << endl;
					exit(-1);
				}
				}
			

			else if (startsWith(str, ".global ")) {
				if (currentSection == "") {
					str = regex_replace(str, regex(".global "), "");
					str = regex_replace(str, regex(" "), "");
					vector<string> strs = split(str, ",");
					for (string s : strs) {
						symtable.setGlobal(s);
					}
				}
				else {
					cout << "Error: Global directive must not be within a section - line " << currentLine << "." << endl;
					exit(-1);
				}
			}

			else if (startsWith(str, ".equ ")) {
					str = regex_replace(str, regex(".equ "), "");
					str = regex_replace(str, regex(" "), "");
					vector<string> strs = split(str, ",");
					if (strs.size() > 2) {
						cout << "Error: Invalid form of equ directive - line " << currentLine << "." << endl;
						exit(-1);
					}
					bool phrase = false;
					Symbol* symbol = nullptr;
					for (string s : strs) {
						if (!phrase) {
							symtable.insert(Symbol(s, "ABS", 0, "local", true));
							symbol = symtable.findSymbol(s);
							phrase = true;
						}
						else {
							s = regex_replace(s, regex("\\-"),"+-");
							vector<string> vals = split(s, "+");
							for (string val : vals) {
								bool minus = false;
								if(val!=""){
									if (startsWith(val, "-")) {  minus = true; val = regex_replace(val, regex("-"), ""); }

									 if (regex_match(val, regex("[[:digit:]]+"))) {
										if (minus) symbol->setOffset(symbol->getOffset() - stoi(val));
										else symbol->setOffset(symbol->getOffset() + stoi(val));
									}

									else if (regex_match(val, regex("0x[ABCDEFabcdef0-9]{1,4}"))) {
										val = regex_replace(val, regex("0x"), "");
										val = toUpper(val);
										if (startsWith(val, "F")) while (val.length() < 8) val = "F" + val;
										if (minus) symbol->setOffset(symbol->getOffset() - hexToDec(val));
										else symbol->setOffset(symbol->getOffset() + hexToDec(val));
									}

									else if (regex_match(val, regex("[a-zA-Z0-9]+"))) {
									if (symtable.isDefined(val) == true) {
										if (minus) {
											symbol->setOffset(symbol->getOffset() - symtable.getOffset(val));
											if(symtable.getSection(val)!="ABS") symbol->removeFromIoC(symtable.getSection(val));
										}
										else {
											symbol->setOffset(symbol->getOffset() + symtable.getOffset(val));
											if (symtable.getSection(val) != "ABS") symbol->addToIoC(symtable.getSection(val));
										}
									}
									else {
										symbol->setDefined(false);
										symbol->numOfPatches++;
										symtable.addBackpatchingRecord(val, symbol->getName(), 0, 0, 0, minus);
									}
									}

									else {
										cout << "Error: Symbol " << val << " specified in equ directive is not in a valid format - line " << currentLine << "." << endl;
										exit(-1);
									}

								}}
						}}
					if (symbol->getIoC() != 0 && symbol->getIoC() != 1 && symbol->numOfPatches==0) {
						cout << "Error: Symbol " << symbol->getName() << " specified in equ directive has invalid definition (invalid index of classification) - line " << currentLine << "." << endl;
						exit(-1);
					}
					else if(symbol->numOfPatches == 0){
						symbol->setSection(symbol->getIoCSection());
					}
			}
			else {
				cout << "Error: Unrecognized command on line " << currentLine << "." << endl;
				exit(-1);
			}
		}

		

		else if(regex_match(str, regex("[a-zA-Z0-9]+\\:"))){
			if (symtable.insert(new Symbol(regex_replace(str, regex(":"), ""), currentSection, currentSectionSize, "local", true))!=0) {
				cout << "Error: Redeclaration of symbol " << str << " on line " << currentLine << "." << endl;
				exit(-1);
			}
		}


		else if(split(str, ":").size() >= 2) {
			vector<string> strs = split(str, ":"); int i=0;
			for (string s : strs) {
				i++;
				if (regex_match(s, regex("[a-zA-Z0-9]+"))) {
					if (symtable.insert(Symbol(s, currentSection, currentSectionSize, "local", true)) != 0) {
						cout << "Error: Redeclaration of symbol " << s << " on line " << currentLine << "." << endl;
						exit(-1);
					}
				}
				else {
					cout << "Error: Symbol has invalid name - " << s << " on line " << currentLine << "." << endl;
					exit(-1);
				}
				if (i == strs.size()-1) break;
			}
			string laststr = strs.at(strs.size()-1);
			if (laststr != "") {
				if (startsWith(laststr, ".byte ") || startsWith(laststr, ".word ") || startsWith(laststr, ".skip ")) {
					allocateMemory(laststr);
				}
				else{
					int ins_type = validInstruction(laststr);
					if (ins_type == 0) {
						cout << "Error: Invalid instruction on line " << currentLine << "." << endl;
						exit(-1);
					}
					else if (currentSection == "") {
						cout << "Error: Instructions must be specified within a section, line - " << currentLine << "." << endl;
						exit(-1);
					}
					else if (ins_type == 1) {
						allocateInstruction1(laststr);
					}
					else if (ins_type == 2) {
						allocateInstruction2(laststr);
					}
					else if (ins_type == 3) {
						allocateInstruction3(laststr);
					}
					else if (ins_type == 4) {
						allocateInstruction4(laststr);
					}
				}
			}
		}

		else if(str!=""){
			int ins_type = validInstruction(str);
			if (ins_type == 0) {
				cout << "Error: Invalid instruction on line - " << currentLine << "." << endl;
				exit(-1);
			}
			else if (currentSection == "") {
				cout << "Error: Instructions must be specified within a section, line - " << currentLine << "." << endl;
				exit(-1);
			}
			else if (ins_type == 1) {
				allocateInstruction1(str);
			}
			else if (ins_type == 2) {
				allocateInstruction2(str);
			}
			else if (ins_type == 3) {
				allocateInstruction3(str);
			}
			else if (ins_type == 4) {
				allocateInstruction4(str);
			}
			}
		
	}
	symtable.equPatch();
	symtable.backpatch();
	//symtable.show();
	symtable.checkUndefined();
	outputFile << symtable;
	inputFile.close();
	outputFile.close();
	cout << "File successfully processed - output: " << output << endl;
	}
}



//syntax as.exe -o out.o in.s
int main(int argc, char** argv) {
	string input;
	string output;
	if (argc != 4) {
		cout << "Error: Invalid arguments." << endl;
		cout << "Usage: as.exe -o output.o input.s" << endl;
		return -1;
	}
	else {
		if(((string)(argv[1])).compare("-o")!=0){
			cout << "Error: Invalid arguments." << endl;
			cout << "Usage: as.exe -o output.o input.s" << endl;
			return -1;
		}
		output = argv[2];
		input = argv[3];
		process(input, output);
	}
	return 0;
}