#pragma once
#include "MidCodeGen.h"
#include "SymbolTable.h"
#include "Error.h"	
#include "Instruction.h"
#include <list>
#include <sstream>

using namespace std;


class mipsObjCode {
private:
	list<string> data;
	list<string> text;
	SymbolTable &tab;
	vector<Quadruples> &midCodes;
	list<Instruction> ins;

	int string_count = 0;
	int paraStack = 4;	// first 4 bytes store $ra, 
	int pc = 0;
	int stack = 0;
	
public:
	void translate();

	void calculate(Operator op, SymbolItem* dst, SymbolItem*src1, SymbolItem*src2);

	void assign(Operator op, SymbolItem* dst, SymbolItem*value, SymbolItem*offset);

	void compare(Operator op, SymbolItem* dst, SymbolItem*src1, SymbolItem*src2);

	void jump(Operator op, SymbolItem* label, SymbolItem* s1, SymbolItem*s2);

	void call(SymbolItem* fun);

	void ret(SymbolItem* fun, SymbolItem* retItem);
	void mark(SymbolItem* fun);
	void storePara(SymbolItem* s1);
	void loadReg(SymbolItem *src, string reg);
	void storeMem(string reg, SymbolItem *dst);
	void read(SymbolItem* item);
	void print(SymbolItem* item);

	void add(MipsCode _op) {
//		ins.push_back(Instruction(_op));
	}
	void add(MipsCode _op, string _des) {
//		ins.push_back(Instruction(_op, " " + _des));
	}
	void add(MipsCode _op, string _des, string _src1) {
	//	ins.push_back(Instruction(_op, " " + _des, "," + _src1));
	}
	void add(MipsCode _op, string _des, string _src1, string _src2) {
/*		if (_op == MipsCode::sw || _op == MipsCode::lw)
			ins.push_back(Instruction(_op, " " + _des, "," + _src1, "(" + _src2 + ")"));
		else
			ins.push_back(Instruction(_op, " " + _des, "," + _src1, "," + _src2));
	*/
	}


	string int2string(int i) {
		stringstream ss;
		ss << i;
		return ss.str();
	}


	mipsObjCode(SymbolTable &tab, vector<Quadruples>&codes);
	~mipsObjCode();
};

