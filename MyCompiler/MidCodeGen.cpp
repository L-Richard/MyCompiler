#include "pch.h"
#include "MidCodeGen.h"
#include <sstream>
#include <fstream>

MidCodeGen::MidCodeGen(SymbolTable&tab) :tab(tab)
{
}


MidCodeGen::~MidCodeGen()
{
}


Operator MidCodeGen::symbol2Operator(Symbol sy) {
	switch (sy) {
	case Symbol::minus: return Operator::minus;
	case Symbol::plus:	return Operator::plus;
	case Symbol::times:	return Operator::times;
	case Symbol::slash:	return Operator::slash;
	case Symbol::eql:	return Operator::eql;
	case Symbol::neq:	return Operator::neq;
	case Symbol::gtr:	return Operator::gtr;
	case Symbol::geq:	return Operator::geq;
	case Symbol::les:	return Operator::les;
	case Symbol::leq:	return Operator::leq;
	default:			return Operator::notOp;
	}
}
Operator MidCodeGen::op_lr(Operator op) {
	// only logical operator need left right change
	switch (op) {
	case Operator::eql:
	case Operator::neq: return op;
	case Operator::gtr:	return Operator::les;
	case Operator::geq:	return Operator::leq;
	case Operator::les:	return Operator::gtr;
	case Operator::leq: return Operator::geq;
	default:			return Operator::notOp;
	}
}

SymbolItem* MidCodeGen::genTemp(Type t) {
	stringstream ss;
	ss << "!Temp_" << tmp_n++;
	SymbolItem* r = new SymbolItem();
	r->typ = t;
	r->obj = tmp;
	r->ident_name = ss.str();
	tab.enterTemp(r);
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
	r->typ = typ;
	r->obj = ObjectiveType::constty;
	r->ident_name = ss.str();
	r->addr = value;
	return r;
}

SymbolItem* MidCodeGen::genLabel(labelType l) {
	SymbolItem * r = new SymbolItem();
	stringstream ss;
	ss << label2str(l) << label_n++;
	r->ident_name = ss.str();
	r->lab = l;
	r->obj = ObjectiveType::label;
	r->addr = 0;
	return r;
}

void MidCodeGen::setLabel(SymbolItem* label) {
	if (this->con == ConditionOptim::falseCon) {
		return;
	}
	Quadruples tmp;
	tmp.op = Operator::setLabel;
	tmp.dst = label;
	midCodes.push_back(tmp);
}

void MidCodeGen::emit(Operator op, SymbolItem* dst, SymbolItem* src1, SymbolItem* src2) {
	if (this->con == ConditionOptim::falseCon) {
		return;
	}
	Quadruples tmp;
	tmp.op = op;
	tmp.src1 = src1;
	tmp.src2 = src2;
	tmp.dst = dst;
	midCodes.push_back(tmp);
}

void MidCodeGen::print(string filename, vector<Quadruples> &midCodes) {
	ofstream  outfile;
	outfile.open(filename);
	for (auto item = midCodes.begin(); item != midCodes.end(); item++) {
		outfile.width(15);
		outfile << op2Str(item->op);
		if (item->src1 != NULL) {
			outfile.width(15);
			outfile << item->src1->ident_name;
		}
		else {
			outfile.width(15);
			outfile << " ";
		}
		if (item->src2 != NULL) {
			outfile.width(15);
			outfile << item->src2->ident_name;
		}
		else {
			outfile.width(15);
			outfile << " ";
		}
		if (item->dst != NULL) {
			outfile.width(15);
			outfile << item->dst->ident_name;
		}
		else {
			outfile.width(15);
			outfile << " ";
		}
		outfile << endl;
	}
}

string MidCodeGen::op2Str(Operator op) {
	switch (op) {
	// calculate
	case Operator::assignOp:return "assign";
	case Operator::minus:	return "minus";
	case Operator::plus:	return "plus";
	case Operator::times:	return "times";
	case Operator::slash:	return "slash";
	case Operator::neg:		return "neg";
	// branch 
	case Operator::gtr:		return "gtr";
	case Operator::geq:		return "geq";
	case Operator::les:		return "les";
	case Operator::leq:		return "leq";
	case Operator::eql:		return "eql";
	case Operator::neq:		return "neq";
	// jump
	case Operator::jmp:		return "jmp";
	case Operator::jez:		return "jez";
	case Operator::setLabel:return "setLabel";
	// call function
	case Operator::save:	return "save";
	case Operator::stPara:	return "stPara";
	case Operator::call:	return "call";
	case Operator::st$ra:	return "st$ra";
	case Operator::returnOp:return "return";
	case Operator::stRetVal:return "stRetVal";
	case Operator::restore: return "restore";

	case Operator::arrLd:	return "arrLd";
	case Operator::arrSt:	return "arrSt";
		
	case Operator::read:	return "read";
	case Operator::print:	return "print";
	default:				return "foo_Operator";
	}
}