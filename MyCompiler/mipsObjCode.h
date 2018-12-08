#pragma once
#include "MidCodeGen.h"
#include "SymbolTable.h"
#include "Error.h"	
#include "Instruction.h"
#include <list>
#include <map>
#include <sstream>
#include <fstream>

using namespace std;


class mipsObjCode {
private:
	list<string> data;
	list<string> text;
	SymbolTable &tab;
	vector<Quadruples> &midCodes;
	list<Instruction> ins;
	map<string, string> strTab;	// map<string content, string name>

	int string_count = 0;
	// int paraStack = 4;	// first 4 bytes store $ra, 
	int pc = 0;
	int stack = 0;

public:
	void translate();
	void enterDataSeg();

	string loadReg(SymbolItem *src, string reg, bool fp = false);
	void storeMem(string reg, SymbolItem *dst);

	void calculate(Operator op, SymbolItem* dst, SymbolItem*src1, SymbolItem*src2);
	void assign(SymbolItem* dst, SymbolItem*value);

	void compare(Operator op, SymbolItem* dst, SymbolItem*src1, SymbolItem*src2);
	void jump(Operator op, SymbolItem* label, SymbolItem* s1, SymbolItem*s2);
	void setLab(SymbolItem* label);

	void save(SymbolItem* fun1, SymbolItem* fun2);
	void storePara(SymbolItem* paras);
	void call(SymbolItem* fun);
	// st$ra
	void ret(SymbolItem* fun, SymbolItem* retItem);
	// void storeRetVal(SymbolItem* item);
	void restore(SymbolItem* fun);

	void arrayLoad(SymbolItem* dst, SymbolItem* offset, SymbolItem* base);
	void arrayStore(SymbolItem* value, SymbolItem* offset, SymbolItem* base);

	void read(SymbolItem* item);
	void print(SymbolItem* str, SymbolItem* exp);


	void add(MipsCode _op) {
		ins.push_back(Instruction(_op));
	}
	void add(MipsCode _op, string _des) {
		ins.push_back(Instruction(_op, " " + _des));
	}
	void add(MipsCode _op, string _des, string _src1) {
		ins.push_back(Instruction(_op, " " + _des, ", " + _src1));
	}
	void add(MipsCode _op, string _des, string _src1, string _src2) {
		if (   _op == MipsCode::sw || _op == MipsCode::sb
			|| _op == MipsCode::lw || _op == MipsCode::lb)
			ins.push_back(Instruction(_op, " " + _des, ", " + _src1, "(" + _src2 + ")"));
		else
			ins.push_back(Instruction(_op, " " + _des, ", " + _src1, ", " + _src2));
	}
	   	
	string int2string(int i) {
		stringstream ss;
		ss << i;
		return ss.str();
	}
	void fatal(string fun, string msg, SymbolItem* item = NULL) {
		cout << "fatal in mipsObjCode::" << fun << ": "
			<< msg << ". ";
		if (item != NULL) {
			cout << item->ident_name;
		}
		cout << endl;
		exit(0);
	}

	void printObjFile(string fileName) {
		ofstream outfile;
		outfile.open(fileName, ios::out | ios::trunc);
		for (auto item = data.begin(); item != data.end(); item++)
			outfile << *item << endl;
		for (auto item = text.begin(); item != text.end(); item++)
			outfile << *item << endl;
		outfile.close();
	}

	mipsObjCode(SymbolTable &tab, vector<Quadruples>&codes);
	~mipsObjCode();
};

