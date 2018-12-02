#pragma once
#include "SymbolTable.h"
#include <vector>

using namespace std;

enum Operator {
	notOp,
	assignOp,
	minus, plus, times, slash, neg,
	gtr, geq, les, leq, eql, neq,
	jmp, jeq, jne, jez, 
	setLabel,
						// jmp: no condition
						// jeq: jump if condition is false;
	call, returnOp, mark, stPara, 
	load, store,	// 
	arrLd, arrSt,		// array read and write
	write, print,
};

enum StandType {
	booltp, inttp, chartp, 
	stringtp, nottp,
};

struct ExpItem {
	StandType valueTp = StandType::nottp;
	int value;						// compute result
	SymbolItem *symbolItem = NULL;	// pointing to var in the sym Table
};

struct Quadruples {
	/* use SymbolItem as expressiong item, and generate a name for temp */
	Operator op;
	SymbolItem * src1 = NULL;
	SymbolItem * src2 = NULL;
	SymbolItem * dst  = NULL;
};

/*
struct SymbolItem {
	std::string ident_name = "";	// global ident
	enum ObjectiveType obj = noty;	// var, const, tmp, fuction, array
	enum Type typ = notp;			// vaule type
	......
	int addr = 0;	// varibale: offset from base of this block
					// constant: char ascii or sigend integer
};
*/


class MidCodeGen {
	vector<Quadruples> midCodes;
	int tmp_n = 0;
	int label_n;
	int if_n = 0;
	int while_n = 0;
	int case_n = 0;
	int switch_n = 0;
	int fun_n = 0;
public:
	Operator symbol2Operator(Symbol sy);

	SymbolItem* genTemp();	// create a temp var to compute  expression

	SymbolItem* genCon(Type typ, int value);

	SymbolItem* genLabel(string s);
	// genLabel: 1.

	void setLabel(SymbolItem* label);

	void emit3(Operator op, SymbolItem* src1, SymbolItem* src2, SymbolItem* dst);
	// compute, 
	
	void emit0(Operator op);

	void emit1(Operator op, SymbolItem*);
	// call, 

	void emit2(Operator op, SymbolItem* src1, SymbolItem* dst);
	// assign, 

	void print();
	string op2Str(Operator op);

	MidCodeGen();

	~MidCodeGen();
};

