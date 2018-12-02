#include "pch.h"
#include "MidCodeGen.h"
#include <sstream>

MidCodeGen::MidCodeGen()
{
}


MidCodeGen::~MidCodeGen()
{
}


Operator MidCodeGen::symbol2Operator(Symbol sy) {
	switch (sy) {
	case Symbol::minus: return Operator::minus;
	case Symbol::plus: return Operator::plus;
	case Symbol::times: return Operator::times;
	case Symbol::slash: return Operator::slash;
	case Symbol::eql: return Operator::eql;
	case Symbol::neq: return Operator::neq;
	case Symbol::gtr: return Operator::gtr;
	case Symbol::geq: return Operator::geq;
	case Symbol::les: return Operator::les;
	case Symbol::leq: return Operator::leq;
	}
}

SymbolItem* MidCodeGen::genTemp() {
	stringstream ss;
	ss << "!Temp_" << tmp_n++;
	SymbolItem* r = new SymbolItem();
	r->typ = Type::inttp;
	r->obj = tmp;
	r->ident_name = ss.str();
	return r;
}

SymbolItem* MidCodeGen::genCon(Type typ, int value) {
	stringstream ss;
	if (typ == Type::inttp) {
		ss << value;
	}
	else {
		ss << (char)value;
	}
	SymbolItem * r = new SymbolItem();
	r->ident_name = ss.str();
	r->addr = value;
}

SymbolItem* MidCodeGen::genLabel(string s) {
	SymbolItem * r = new SymbolItem();
	stringstream ss;
	ss << s << label_n++;
	r->ident_name = ss.str();

	r->obj = ObjectiveType::label;
	r->addr = 0;
	// offset?????????????????????????????????????????????????????????????????????????????????????
}

void MidCodeGen::setLabel(SymbolItem* label) {
	Quadruples tmp;
	tmp.op = Operator::setLabel;
	tmp.src1 = label;
	midCodes.push_back(tmp);
}

void MidCodeGen::emit1(Operator op, SymbolItem* src1) {
	Quadruples tmp;
	tmp.op = op;
	tmp.src1 = src1;
	midCodes.push_back(tmp);
}

void MidCodeGen::emit2(Operator op, SymbolItem* src1, SymbolItem* dst) {
	Quadruples tmp;
	tmp.op = op;
	tmp.src1 = src1;
	tmp.dst = dst;
	midCodes.push_back(tmp);
}

void MidCodeGen::emit3(Operator op, SymbolItem* src1, SymbolItem* src2, SymbolItem* dst) {
	Quadruples tmp;
	tmp.op = op;
	tmp.src1 = src1;
	tmp.src2 = src2;
	tmp.dst = dst;
	midCodes.push_back(tmp);
}

void MidCodeGen::print() {
	for (auto item = midCodes.begin(); item != midCodes.end(); item++) {
		cout << op2Str(item->op);
		if (item->src1 != NULL) {
			cout << item->src1->ident_name;
		}
	}
}

string MidCodeGen::op2Str(Operator op) {
	switch (op) {
	case Operator::assignOp: return "assign";
	case Operator::minus: return "minus";
	case Operator::plus: return "plus";
	case Operator::times: return "times";
	case Operator::slash: return "slash";
	case Operator::neg: return "neg";

	case Operator::gtr: return "gtr";
	case Operator::geq: return "geq";
	case Operator::les: return "les";
	case Operator::leq: return "leq";
	case Operator::eql: return "eql";
	case Operator::neq: return "neq";

	case Operator::jmp: return "jmp";
	case Operator::jeq: return "jeq";
	case Operator::jne: return "jne";
	case Operator::jez: return "jez";
	case Operator::setLabel: return "setLabel";

	case Operator::call: return "call";
	case Operator::returnOp: return "return";
	case Operator::mark: return "mark";
	case Operator::stPara: return "stPara";

	case Operator::load: return "load";
	case Operator::store: return "store";
		
	case Operator::arrSt: return "arrSt";
	case Operator::arrLd: return "arrLd";

	case Operator::write: return "write";
	case Operator::print: return "print";
	}
}