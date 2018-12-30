#pragma once
#include "SymbolTable.h"
#include <vector>

using namespace std;

enum class Operator {
	notOp,
	assignOp,
	minus, plus, times, slash, neg,

	gtr, geq, les, leq, eql, neq,	// if condition is true then jump to label
	jmp, jez, 
	setLabel,
	
	save, stPara, call, 
	st$ra, returnOp, 
	stRetVal, restore,

	arrLd, arrSt,		// 
	read, print,
};

enum class ConditionOptim {
	unknownCon,
	trueCon,
	falseCon,
};

struct Quadruples {
	/* use SymbolItem as expressiong item, and generate a name for temp */
	Operator op;
	SymbolItem * src1 = NULL;
	SymbolItem * src2 = NULL;
	SymbolItem * dst  = NULL;
};

class MidCodeGen {
	vector<Quadruples> midCodes;
	SymbolTable &tab;
	int tmp_n = 0;
	int label_n;
	int if_n = 0;
	int while_n = 0;
	int case_n = 0;
	int switch_n = 0;
	int fun_n = 0;

	/* condition result when condition expression is consist of
		two const value or record switch case compare result
	*/
	ConditionOptim con = ConditionOptim::unknownCon; 

public:
	Operator symbol2Operator(Symbol sy);

	SymbolItem* genTemp(Type t = Type::inttp);	// create a temp var to compute  expression

	SymbolItem* genCon(Type typ, int value);

	SymbolItem* genLabel(labelType l);
	// genLabel: 1.

	void setLabel(SymbolItem* label);

	void emit(Operator op, SymbolItem* dst, SymbolItem* src1, SymbolItem* src2);
	// compute, 
	
	void print();
	string op2Str(Operator op);
	string label2str(labelType l) {
		switch (l) {
		case labelType::main_lb:		return "main_lb";
		case labelType::end_main_lb:	return "end_main_lb";
		case labelType::fun_lb:			return "fun_lb";
		case labelType::end_fun_lb:		return "end_fun_lb";
		case labelType::while_lb:		return "while_lb";
		case labelType::end_while_lb:	return "end_while_lb";
		case labelType::end_if_lb:		return "end_if_lb";
		case labelType::end_case_lb:	return "end_case_lb";
		case labelType::end_switch_lb:	return "end_switch_lb";
		default:						return "foo_label";
		}
	}
	void setFalseCon() {
		this->con = ConditionOptim::falseCon;
	}
	void setTrueCon() {
		this->con = ConditionOptim::trueCon;
	}
	void restoreCon() {
		this->con = ConditionOptim::unknownCon;
	}
	ConditionOptim getCon() {
		return this->con;
	}
	void eraseLabel(SymbolItem * label) {
		for (auto item = midCodes.begin(); item != midCodes.end(); item++) {
			if (item->op == Operator::setLabel && item->dst->ident_name == label->ident_name) {
				midCodes.erase(item);
				break;
			}
		}
	}


	vector<Quadruples>& getMidCodes() {
		return midCodes;
	}
	MidCodeGen(SymbolTable &tab);

	~MidCodeGen();
};

