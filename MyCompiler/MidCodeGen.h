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
	SymbolItem * dst  = NULL;
	SymbolItem * src1 = NULL;
	SymbolItem * src2 = NULL;
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
	
	// optimization
	/* condition result when condition expression is consist of
		two const value or record switch case compare result
	*/
	ConditionOptim con = ConditionOptim::unknownCon; 
	// blank if optimization, this three index means offset after 
	// midCodes.begin()
	int opt_if_start_index = 0;
	int opt_if_end_index = 0;
	int opt_if_statement_index = 0;

public:
	static void print(string filename, vector<Quadruples> & codes);	// print midcodes 

	// four functional method
	static Operator symbol2Operator(Symbol sy);
	static Operator op_lr(Operator op);
	static string op2Str(Operator op);
	static string label2str(labelType l) {
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
	
	// generators
	SymbolItem* genTemp(Type t = Type::inttp);	// create a temp var to compute  expression
	SymbolItem* genCon(Type typ, int value);
	SymbolItem* genLabel(labelType l);
	void setLabel(SymbolItem* label);

	// emit middle code 
	void emit(Operator op, SymbolItem* dst, SymbolItem* src1, SymbolItem* src2);
	
	
	/***************************    optimization:    *****************************/
	// assign optimization: remove extra !Temp var
	// call this function after parse.
	void opt_assign(SymbolItem *left_var, SymbolItem *assignRet) {
		const set<Operator> s = { Operator::neg, 
			Operator::minus, Operator::plus, Operator::slash, Operator::times };
		auto item = midCodes.end() - 1;
		// use if to check assign right var.
		if (item->op == Operator::assignOp
			&& item->src1->ident_name == assignRet->ident_name
			&& assignRet->obj == ObjectiveType::tmp) {
			auto lastExp = item - 1;

			if (s.count(lastExp->op)) {
				lastExp->dst = left_var;
				midCodes.erase(item);
				tab.removeLastTemp(assignRet);
			}
		}
	}


	// if statement: remove blank if statement
	void opt_if_record_start() {
		// record current tail midcode index
		opt_if_start_index = midCodes.end()-midCodes.begin();
	}
	void opt_if_record_statement() {
		// record before statement
		opt_if_statement_index = midCodes.end()-midCodes.begin();
	}
	void opt_if_record_end() {
		// record after set end_if label
		opt_if_end_index = midCodes.end()-midCodes.begin();
	}
	void opt_if_optimize() {
		if (opt_if_end_index - opt_if_statement_index == 1) {
			// the difference is 1 means no statement, optimize
			auto item = midCodes.begin() + opt_if_start_index;
			while (item != midCodes.end()) {
				midCodes.erase(item);
				item = midCodes.begin() + opt_if_start_index;
			}
		}
		opt_if_start_index = 0;
		opt_if_statement_index = 0;
		opt_if_end_index = 0;
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
			if (item->op == Operator::setLabel && item->src1->ident_name == label->ident_name) {
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

