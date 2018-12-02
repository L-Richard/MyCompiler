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
const string $t4 = "$t4";
const string $t5 = "$t5";
const string $t6 = "$t6";
const string $t7 = "$t7";

mipsObjCode::mipsObjCode(SymbolTable &tab, vector<Quadruples> &codes) : tab(tab), midCodes(codes) {
	string s = "_str_endl: .assiz \"\n\"";
	data.push_back(s);
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
		case Operator::les:
		case Operator::eql:
		case Operator::neq:
			compare(item->op, item->dst, item->src1, item->src2);
			break;
		case Operator::jmp:
		case Operator::jne:
		case Operator::jez:
			jump(item->op, item->dst, item->src1, item->src2);
			break;
		case Operator::call:
			call(item->src1);	break;
		case Operator::returnOp:
			ret(item->src1, item->src2);	break;
		case Operator::stRa:
			add(MipsCode::sw, $ra, "0", $sp);		//
			break;
		case Operator::mark:
			mark(item->src1);	break;
		case Operator::stPara:
			storePara(item->src1); break;
		case Operator::stRetVal: {
			storeMem($v0, item->src1);
			break;
		}
		case Operator::read:
			read(item->src1);
			break;
		case Operator::print:
			print(item->src1);
			break;
		}
	}
}

void mipsObjCode::loadReg(SymbolItem* src, string reg) {
	/*
		the load_addr_value is true means that:
		_$i =  addr -> use in multi function var parameter.
		else means
		_$i = *addr
	*/
	if (src->obj == ObjectiveType::constValue || src->obj == ObjectiveType::constty) {
		add(MipsCode::li, reg, int2string(src->addr));
		return;
	}
	else if (src->obj == ObjectiveType::varty || src->obj == ObjectiveType::tmp) {
		if (src->typ == Type::chartp) {
			add(MipsCode::lb, reg, int2string(src->addr), $sp);
		}
		else {
			add(MipsCode::lw, reg, int2string(src->addr), $sp);
		}
		return;
	}
	else {
		cout << "fatal: mipsObjCode::storeMem" << endl;
		exit(0);
	}
	/*
	else if (src->obj == ObjectiveType::functy) {
		add(MipsCode::addi, reg, $v0, "0");
		return;
	}
	*/
}

void mipsObjCode::storeMem(string reg, SymbolItem* dst) {
	if (dst->level == 0) {
		// global var
		if (dst->typ == Type::chartp) {
			add(MipsCode::sb, reg, dst->ident_name);
		}
		else if (dst->typ == Type::inttp) {
			add(MipsCode::sw, reg, dst->ident_name);
		}
	}
	else if (dst->obj == ObjectiveType::varty || dst->obj == ObjectiveType::tmp) {
		if (dst->typ == Type::chartp) {
			add(MipsCode::sb, reg, int2string(dst->addr), $sp);
		}
		else if (dst->typ == Type::inttp) {
			add(MipsCode::sw, reg, int2string(dst->addr), $sp);
		}
		return;
	}
	else {
		cout << "fatal: mipsObjCode::storeMem" << endl;
		exit(0);
	}
}

void mipsObjCode::calculate(Operator op, SymbolItem* dst, SymbolItem*src1, SymbolItem*src2) {
	add(MipsCode::note, "calc");
	loadReg(src1, $t0);
	loadReg(src2, $t1);
	if (op == Operator::times) {
		add(MipsCode::mult, $t2, $t0, $t1);
	}
	else if (op == Operator::plus) {
		add(MipsCode::add, $t2, $t0, $t1);
	}
	else if (op == Operator::minus) {
		add(MipsCode::sub, $t2, $t0, $t1);
	}
	else if (op == Operator::slash) {
		add(MipsCode::ddiv, $t0, $t1);
		add(MipsCode::mflo, $t2);
	}
	storeMem($t2, dst);
}

void mipsObjCode::assign(Operator op, SymbolItem* dst, SymbolItem*value, SymbolItem*offset) {
	// offset may be NULL if not array
	loadReg(value, $t7);
	if (offset != NULL) {
		loadReg(offset, $t6);
		add(MipsCode::addi, $t6, $t6, int2string(dst->addr));
		add(MipsCode::sw, $t7, "0", $t6);
	}
	else {
		storeMem($t7, dst);
	}
}

void mipsObjCode::compare(Operator op, SymbolItem* dst, SymbolItem*src1, SymbolItem*src2) {
	loadReg(src1, $t0);
	loadReg(src2, $t1);
	switch (op) {
	case Operator::eql: add(MipsCode::bne, $t0, $t1, dst->ident_name);
	case Operator::neq: add(MipsCode::beq, $t0, $t1, dst->ident_name);
	case Operator::gtr: add(MipsCode::ble, $t0, $t1, dst->ident_name);
	case Operator::geq: add(MipsCode::beq, $t0, $t1, dst->ident_name);
	case Operator::les: add(MipsCode::beq, $t0, $t1, dst->ident_name);
	case Operator::leq: add(MipsCode::beq, $t0, $t1, dst->ident_name);
	}
}

void mipsObjCode::jump(Operator op, SymbolItem* label, SymbolItem* s1, SymbolItem*s2) {
	if (op == Operator::jmp) {
		add(MipsCode::j, label->ident_name);
	}
	else if (op == Operator::jez) {
		loadReg(s1, $t0);
		add(MipsCode::beqz, $t0, label->ident_name);
	}
	else if (op == Operator::jne) {
		loadReg(s1, $t0);
		add(MipsCode::bnez, $t0, label->ident_name);
	}
}

void mipsObjCode::call(SymbolItem* fun) {
	paraStack = 4;
	add(MipsCode::jal, fun->fun_label->ident_name);
}

void mipsObjCode::ret(SymbolItem* fun, SymbolItem* retItem) {
	if (fun->hasRet && retItem != NULL) {
		// has return value in retItem;
		if (fun->typ == Type::chartp) {
			add(MipsCode::lb, $v0, int2string(retItem->addr), $sp);
		}
		else {
			add(MipsCode::lw, $v0, int2string(retItem->addr), $sp);
		}
	}
	// restore memory
	add(MipsCode::lw, $ra, "0", $sp);
	add(MipsCode::addi, $sp, $sp, int2string(fun->totalSize));
}
void mipsObjCode::mark(SymbolItem* fun) {
	add(MipsCode::addi, $sp, $sp, int2string(fun->totalSize));
	// save some registers????????????????
}
void mipsObjCode::storePara(SymbolItem* s1) {
	loadReg(s1, $t0);
	if (s1->typ == Type::chartp) {
		add(MipsCode::sb, $t0, int2string(paraStack), $sp);
		paraStack += CHARSIZE;
	}
	else if (s1->typ == Type::inttp) {
		add(MipsCode::sw, $t0, int2string(paraStack), $sp);
		paraStack += INTSIZE;
	}
}

void mipsObjCode::read(SymbolItem* item) {
	if (item->typ == Type::chartp) {
		add(MipsCode::li, $v0, "12");
	}
	else if (item->typ == Type::inttp) {
		add(MipsCode::li, $v0, "5");
	}
	add(MipsCode::syscall);
	storeMem($v0, item);
}

void mipsObjCode::print(SymbolItem* item) {
	if (item == NULL) {
		add(MipsCode::li, $v0, "4");
		add(MipsCode::la, $a0, "_str_endl");
		add(MipsCode::syscall);

	}
	else if (item->typ == Type::stringtp) {
		string sname;
		stringstream ss;
		ss << "_str_";
		ss << string_count++;
		sname = ss.str();
		ss << ": .assiz \"";
		ss << item->ident_name;
		ss << "\"";
		string s = ss.str();
		data.push_back(s);
		add(MipsCode::li, $v0, "4");
		add(MipsCode::la, $a0, sname);
		add(MipsCode::syscall);
	}
	else if (item->typ == Type::chartp) {
		add(MipsCode::li, $v0, int2string(11));
		if (item->obj == ObjectiveType::constty || item->obj == ObjectiveType::constValue) {
			add(MipsCode::li, $a0, int2string(item->addr));
		}
		else{
			add(MipsCode::lb, $a0, int2string(item->addr), $sp);
		}
		add(MipsCode::syscall);
	}
	else if (item->typ == Type::inttp) {
		add(MipsCode::li, $v0, int2string(1));
		if (item->obj == ObjectiveType::constty || item->obj == ObjectiveType::constValue) {
			add(MipsCode::li, $a0, int2string(item->addr));
		}
		else{
			add(MipsCode::lw, $a0, int2string(item->addr), $sp);
		}
		add(MipsCode::syscall);
	}
}




