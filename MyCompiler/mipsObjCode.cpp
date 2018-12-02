#include "pch.h"
#include "mipsObjCode.h"

const string $sp = "$sp";
const string $fp = "$fp";
const string $s0 = "$s0";
const string $s1 = "$s1";
const string $s2 = "$s2";
const string $ra = "$ra";
const string $v0 = "$v0";
const string $t0 = "$t0";
const string $t1 = "$t1";
const string $t2 = "$t2";
const string $a0 = "$a0";
const string $0  = "$0";
const string $t7 = "$t7";

mipsObjCode::mipsObjCode(SymbolTable &tab, vector<Quadruples> &codes) : tab(tab), midCodes(codes) {
	
}


mipsObjCode::~mipsObjCode()
{
}


void mipsObjCode::translate() {
	for (auto item = midCodes.begin(); item != midCodes.end(); item++) {
		switch (item->op) {
		case Operator::minus:
		case Operator::plus:
		case Operator::times:
		case Operator::slash:
			calculate(item->op, item->dst, item->src1, item->src2);
			break;
		case Operator::assignOp:
			assign(item->op, item->dst, item->src1, item->src2);
			break;
		case Operator::gtr:
		case Operator::geq:
		case Operator::leq:
		case Operator::eql:
		case Operator::neq:
			compare(item->op, item->dst, item->src1, item->src2);
			break;
		case Operator::jmp:
		case Operator::jeq:
		case Operator::jne:
		case Operator::jez:
			jump(item->op, item->dst, item->src1, item->src2);
			break;
		case Operator::call:
			call(item->src1);	break;
		case Operator::returnOp:
			ret();	break;
		case Operator::mark:
			mark();	break;
		case Operator::stPara:
			storePara(item->src1); break;
		case Operator::arrLd:
			arrLoad(item->src1, item->src2, item->dst);
		case Operator::read:
			read();
		case Operator::print:
			print();
		}
	}
}

void mipsObjCode::calculate(Operator op, SymbolItem* dst, SymbolItem*src1, SymbolItem*src2) {

}

void mipsObjCode::assign(Operator op, SymbolItem* dst, SymbolItem*value, SymbolItem*offset) {

}

void mipsObjCode::compare(Operator op, SymbolItem* dst, SymbolItem*src1, SymbolItem*src2) {


}

void mipsObjCode::jump(Operator op, SymbolItem* label, SymbolItem* s1, SymbolItem*s2) {

}

void mipsObjCode::call(SymbolItem* fun) {

}

void mipsObjCode::ret() {

}
void mipsObjCode::mark() {

}
void mipsObjCode::storePara(SymbolItem* s1) {

}
void mipsObjCode::loadVar() {

}
void mipsObjCode::storeVar() {

}
void mipsObjCode::arrStore() {

}
void mipsObjCode::arrLoad(SymbolItem* arr, SymbolItem*offset, SymbolItem*dst) {

}
void mipsObjCode::read() {

}
void mipsObjCode::print() {

}




