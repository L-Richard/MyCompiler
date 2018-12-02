#pragma once
#include "MidCodeGen.h"
#include "SymbolTable.h"
#include "Error.h"
#include <list>

using namespace std;

class mipsObjCode {
private:
	list<string> data;
	list<string> text;
	SymbolTable &tab;
	vector<Quadruples> &midCodes;

	int pc = 0;
	int stack = 0;
	
public:
	void translate();

	void calculate(Operator op, SymbolItem* dst, SymbolItem*src1, SymbolItem*src2);

	void assign(Operator op, SymbolItem* dst, SymbolItem*value, SymbolItem*offset);

	void compare(Operator op, SymbolItem* dst, SymbolItem*src1, SymbolItem*src2);

	void jump(Operator op, SymbolItem* label, SymbolItem* s1, SymbolItem*s2);

	void call(SymbolItem* fun);

	void ret();
	void mark();
	void storePara(SymbolItem* s1);
	void loadVar();
	void storeVar();
	void arrStore();
	void arrLoad(SymbolItem* arr, SymbolItem*offset, SymbolItem*dst);
	void read();
	void print();

	mipsObjCode(SymbolTable &tab, vector<Quadruples>&codes);
	~mipsObjCode();
};

