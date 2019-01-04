#include "pch.h"
#include "mipsObjCode.h"
#include "Const.h"
const string $sp = "$sp";
const string $fp = "$fp";
const string $s0 = "$s0";
const string $s1 = "$s1";
const string $s2 = "$s2";
const string $ra = "$ra";
const string $v0 = "$v0";
const string $t0 = "$t0";
const string $t1 = "$t1";
const string $a0 = "$a0";
const string $0  = "$0";
const string $t4 = "$t4";
const string $t5 = "$t5";
const string $t6 = "$t6";
const string $t7 = "$t7";

mipsObjCode::mipsObjCode(SymbolTable &tab, vector<Quadruples> &codes) : tab(tab), midCodes(codes) {
}
mipsObjCode::~mipsObjCode()
{
}


void mipsObjCode::peepHole() {
	for (auto item = ins.begin(); item != ins.end(); item++) {
		if (item->getOp() == "sw" || item->getOp() == "lw") {
			auto tmp = next(item, 1);
			while (tmp != ins.end()
				&& tmp->getOp() == "lw"
				&& tmp->getDst() == item->getDst()
				&& tmp->getS1() == item->getS1()
				&& tmp->getS2() == item->getS2()) {

				ins.erase(tmp);
				tmp = next(item, 1);
			}
		}
		if (item->getOp() == "j") {
			bool remove_j = false;
			for (auto tmp = next(item, 1); tmp != ins.end() && tmp->getOp() == ""; tmp++) {
				if (tmp->getDst() == item->getDst() + ":") {
					remove_j = true;
				}
			}
			if (remove_j) {
				// remove item
				if (item == ins.begin()) {
					ins.erase(item);
					item = ins.begin();
				}
				else {
					item--;
					ins.erase(next(item, 1));
				}
			}
		}
	}
}

void mipsObjCode::translate() {
	enterDataSeg();
	for (auto item = midCodes.begin(); item != midCodes.end(); item++) {
		current_code = &*item;
		switch (item->op) {
		case Operator::assignOp:
			assign(item->dst, item->src1);
			break;
		case Operator::minus:
		case Operator::plus:
		case Operator::times:
		case Operator::slash:
		case Operator::neg:
			calculate(item->op, item->dst, item->src1, item->src2);
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
		case Operator::jez:
			jump(item->op, item->dst, item->src1, item->src2);
			break;
		case Operator::setLabel:
			setLab(item->dst);						break;

		// call function 
		case::Operator::save:
			save(item->src1, item->src2);	break;
		case Operator::stPara:
			storePara(item->src1, item->src2);			break;
		case Operator::call:
			call(item->dst);				break;
		case Operator::st$ra:
			add(MipsCode::sw, $ra,"0",$sp);	break;
		case Operator::returnOp:
			ret(item->dst, item->src1);	break;
		case Operator::stRetVal:
			if (regTab.count(item->dst->allocated_reg)) {	// store return value， don't need restore
				save_restore.erase(regTab[item->dst->allocated_reg]);
			}
			storeMem($v0, item->dst);		
			break;
		case Operator::restore:
			restore(item->src1);			break;

		case Operator::arrLd:
			arrayLoad(item->dst, item->src1, item->src2);
			break;
		case Operator::arrSt:
			arrayStore(item->dst, item->src1, item->src2);
			break;
			
		case Operator::read:
			read(item->dst);				break;
		case Operator::print:
			print(item->src1, item->src2);				break;
		}
	}
	// optimize:
	this->peepHole();
}

void mipsObjCode::enterDataSeg() {
	map<string, SymbolItem*> &symTab = tab.getTab();
	data.push_back(".data");
	for (auto item = symTab.begin(); item != symTab.end(); item++) {
		if (item->second->obj == ObjectiveType::varty || item->second->obj == ObjectiveType::arrayty) {
			item->second->ident_name = "_data_" + item->second->ident_name;
			stringstream ss;
			ss << "\t.align 2" << endl;
			ss << "\t" << item->second->ident_name;
			ss << ": .space " << item->second->varSize;
			data.push_back(ss.str());
		}
	}
	string s = "\t_str_endl: .asciiz \"\\n\"";
	data.push_back(s);
}

void mipsObjCode::saveReg(SymbolItem* var, bool useFp) {
	// 调用函数前save，
	string stackPointer = useFp ? $fp : $sp;
	if (var->saved == false && var->allocated_reg != "") {
		add(MipsCode::sw, var->allocated_reg, int2string(var->addr), stackPointer);
		var->saved = true;
	}
}

string mipsObjCode::loadReg(SymbolItem* src, string reg, bool useFp) {
	// get src's value from memory to a certain register 
	//				or from src's register
	/********************* optimiszation registers *******************************************/
	if (src->allocated_reg != "") {
		reg = src->allocated_reg;
		if (src->isUsingReg) {	// isUsing 表示没有其他变量占用寄存器
			if (src->isPara) {
				if (regTab.count(src->allocated_reg) && regTab[src->allocated_reg] != src) {
					regTab[src->allocated_reg]->isUsingReg = false;
				}
				regTab[src->allocated_reg] = src;
			}
			return src->allocated_reg;
		}
		// update reg table
		if (regTab.count(reg) && regTab[reg] != src) {
			regTab[reg]->isUsingReg = false;
//			if (regTab[reg]->isGlobal && !regTab[reg]->saved) {
//				// 保存同一个函数内的变量(可能也不必要)，不是同一个函数，调用时已保存
//				saveReg(regTab[reg]);
//			}
		}
		regTab[reg] = src;
		src->isUsingReg = true;
		if (src->initial) {	// 参数不可以在这里返回，需要initial=false
			src->initial = false;
			return src->allocated_reg;
		}
	}
	/****************************************************************************/

	string stackPointer = useFp ? $fp : $sp;
	if (src->obj == ObjectiveType::constty) {
		add(MipsCode::li, reg, int2string(src->addr));
		return reg;
	}
	else if (src->obj == ObjectiveType::varty || src->obj == ObjectiveType::tmp) {
		if (src->typ == Type::chartp) {
			if (src->level == 1)
				add(MipsCode::lb, reg, int2string(src->addr), stackPointer);
			else
				add(MipsCode::lb, reg, src->ident_name);
		}
		else {
			if (src->level == 1)
				add(MipsCode::lw, reg, int2string(src->addr), stackPointer);
			else
				add(MipsCode::lw, reg, src->ident_name);
		}
		return reg;
	}
	else {
		fatal("loadReg", "obj error", src);
	}
	return reg;
}

void mipsObjCode::storeMem(string reg, SymbolItem* dst) {
	// put dst's value into reg
	// store var value to register or memory
	if (dst->obj == ObjectiveType::constty || dst == NULL)
		fatal("storeMem", "dst obj error or dst is null", dst);

	/********************* optimiszation registers *******************************************/
	if (dst->allocated_reg != "") {
		dst->isUsingReg = true;
		dst->initial = false;
		dst->saved = false;
		if (regTab.count(dst->allocated_reg) && regTab[dst->allocated_reg] != dst) {
			// saveReg(regTab[dst->allocated_reg]);
			regTab[dst->allocated_reg]->isUsingReg = false;
		}
		regTab[dst->allocated_reg] = dst;
		if (dst->allocated_reg != reg) {
			add(MipsCode::move, dst->allocated_reg, reg);
		}
		return;
	}
	/****************************************************************************/

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
		fatal("storeMem", "obj error", dst);
	}
}

//  ok 带常数计算可以再优化  ???????????????????????????????????????????????????????????????????????????????????
void mipsObjCode::calculate(Operator op, SymbolItem* dst, SymbolItem*src1, SymbolItem*src2) {
	add(MipsCode::note, "calc");
	
	string regDst = loadReg(dst, $t0);
	string reg0 = loadReg(src1, $t0);
	string reg1 = "";
	if (src2 != NULL)
		reg1 = loadReg(src2, $t1);
	if (op == Operator::times) {
		add(MipsCode::mul, regDst, reg0, reg1);
	}
	else if (op == Operator::plus) {
		add(MipsCode::add, regDst, reg0, reg1);
	}
	else if (op == Operator::minus) {
		add(MipsCode::sub, regDst, reg0, reg1);
	}
	else if (op == Operator::slash) {
		add(MipsCode::ddiv, reg0, reg1);
		add(MipsCode::mflo, regDst);
	}
	else if (op == Operator::neg) {
		add(MipsCode::sub, regDst, $0, reg0);
	}
	storeMem(regDst, dst);
}

// **** 常数赋值直接li  ???????????????????????????????????????????????????????????????????????????????????
void mipsObjCode::assign(SymbolItem* dst, SymbolItem*value) {
	add(MipsCode::note, "assign");
	string reg = loadReg(value, $t0);
	storeMem(reg, dst);
	add(MipsCode::note, "end assign");
}

void mipsObjCode::compare(Operator op, SymbolItem* dst, SymbolItem*src1, SymbolItem*src2) {
	/* src1, src2 has at most one const, 
	otherwise we can optimize the code when generate mid code
	*/ 
	string reg0 = loadReg(src1, $t0);
	string reg1 = "";	// here reg1 can be a number or a register
	if (src2->obj == ObjectiveType::constty)
		reg1 = int2string(src2->addr);
	else
		reg1 = loadReg(src2, $t1);

	switch (op) {
	case Operator::eql: add(MipsCode::bne, reg0, reg1, dst->ident_name); break;
	case Operator::neq: add(MipsCode::beq, reg0, reg1, dst->ident_name); break;
	case Operator::gtr: add(MipsCode::ble, reg0, reg1, dst->ident_name); break;
	case Operator::geq: add(MipsCode::blt, reg0, reg1, dst->ident_name); break;
	case Operator::les: add(MipsCode::bge, reg0, reg1, dst->ident_name); break;
	case Operator::leq: add(MipsCode::bgt, reg0, reg1, dst->ident_name); break;
	default: fatal("compare", "error op: not logical operator");
	}
}

void mipsObjCode::jump(Operator op, SymbolItem* label, SymbolItem* s1, SymbolItem*s2) {
	if (op == Operator::jmp) {
		add(MipsCode::j, label->ident_name);
	}
	else if (op == Operator::jez) {
		string reg = loadReg(s1, $t0);
		add(MipsCode::beqz, reg, label->ident_name);
	}
}

void mipsObjCode::setLab(SymbolItem* label) {
	if (label->lab == labelType::fun_lb || label->lab == labelType::main_lb) {
		current_fun = label->funItem;
		regTab.clear();
	}
	add(MipsCode::label, label->ident_name+":");
}

void mipsObjCode::save(SymbolItem* fun1, SymbolItem* fun2) {
	// save some registers used by fun1, and move $sp for fun2
	add(MipsCode::note, "save");
	add(MipsCode::move, $fp, $sp);
	add(MipsCode::subi, $sp, $sp, int2string(fun2->totalSize));
	for (auto item : regTab) {
		if (fun1->link->count((item.second->ident_name)) && (*(fun1->link))[item.second->ident_name] == item.second) {
			save_restore.insert(item.second);
		}
	}
	for (auto item : fun1->paraItems) {
		save_restore.insert(item);
	}
	for (auto item : save_restore) {
		saveReg(item, true);
	}
}

void mipsObjCode::storePara(SymbolItem* paras, SymbolItem* fun) {
	/* paras->paras record the parameters, and save() must be called before this function
	   optimize: 1. item is register. 
				 2. donnot need to store into memory
	*/
	if (paras == NULL || paras->obj != ObjectiveType::paras) {
		fatal("storePara", "obj error or paras is NULL", paras);
	}
	int paraStack = 4;
	add(MipsCode::note, "storePara");
	for (int i = 0; i < paras->paras.size(); i++) {
		/*
		string reg = loadReg(paras->paras[i], $t0, true);
		add(MipsCode::sw, reg, int2string(paraStack), $sp);
		paraStack += INTSIZE;
		*/
		/*********************/
		auto actParas = fun->paraItems;
		if (actParas[i]->allocated_reg != "") {
			// actParas[i]->isUsingReg = true;
			// actParas[i]->saved = false;
			// actParas[i]->initial = false;
			if (actParas[i]->allocated_reg == paras->paras[i]->allocated_reg && paras->paras[i]->isUsingReg) {

			}
			else if (paras->paras[i]->allocated_reg != "" && paras->paras[i]->isUsingReg) {
				add(MipsCode::move, actParas[i]->allocated_reg, paras->paras[i]->allocated_reg);
			}
			else {
				// load if this two vars using different regs or paras.paras is not using reg.
				if (paras->paras[i]->obj != ObjectiveType::constty) {
					add(MipsCode::lw, actParas[i]->allocated_reg, int2string(paras->paras[i]->addr), $fp);
				}
				else {
					add(MipsCode::li, actParas[i]->allocated_reg, int2string(paras->paras[i]->addr));
				}
			}
			if (regTab.count(actParas[i]->allocated_reg)) {
				regTab[actParas[i]->allocated_reg]->isUsingReg = false;
				saveReg(regTab[actParas[i]->allocated_reg], true);
			}
			// regTab[actParas[i]->allocated_reg] = actParas[i];	// 更新寄存器表
		}
		else {
			string reg = loadReg(paras->paras[i], $t0, true);
			add(MipsCode::sw, reg, int2string(paraStack), $sp);
		}
		paraStack += INTSIZE;
		/*********************/
	}
	/*for (auto item = paras->paras.begin(); item != paras->paras.end(); item++) {
		string reg = loadReg(*item, $t0, true);
		add(MipsCode::sw, reg, int2string(paraStack), $sp);
		paraStack += INTSIZE;
	}*/
	add(MipsCode::note, "end storePara");
}

void mipsObjCode::call(SymbolItem* fun) {
	// isUsingReg = false
	for (auto item : regTab) {
		item.second->isUsingReg = false;
	}
	add(MipsCode::note, "call");
	// paraStack = 4;
	add(MipsCode::jal, fun->fun_label->ident_name);
}

//?????????????? lw $ra ?????????????????????????????????????????????????????????????????????
void mipsObjCode::ret(SymbolItem* fun, SymbolItem* retItem) {
	if (fun->hasRet && retItem != NULL) {
		// has return value in retItem;
		if (retItem->obj == ObjectiveType::constty)
			add(MipsCode::li, $v0, int2string(retItem->addr));
		else {
		/*******************************************************************/
			string retVar = loadReg(retItem, $v0);
			if (retVar != $v0) {
				add(MipsCode::move, $v0, retVar);
			}
		/*******************************************************************/
			//add(MipsCode::lw, $v0, int2string(retItem->addr), $sp);
		}
	}
	//for (auto item : regTab) {
	//	item.second->isUsingReg = false;
	//}
	// restore memory
	add(MipsCode::lw, $ra, "0", $sp);
	add(MipsCode::addi, $sp, $sp, int2string(fun->totalSize));
	add(MipsCode::jr, $ra);
}

void mipsObjCode::restore(SymbolItem* fun) {
	add(MipsCode::note, "restore");
	// load some register value
	for (auto var : save_restore) {
		if (var->allocated_reg != "") {
			add(MipsCode::lw, var->allocated_reg, int2string(var->addr), $sp);
			regTab[var->allocated_reg] = var;
			var->isUsingReg = true;
		}
	}
	save_restore.clear();
	add(MipsCode::note, "end restore");
}

void mipsObjCode::arrayLoad(SymbolItem* dst, SymbolItem* offset, SymbolItem* base) {
	/*  dst <= base[off]
	_offset = base->addr + size(base->typ) * offset + $sp
	dst -> $t0
	store($t0, dst);
	global/local, int/char, consty/var
	*/
	string reg0 = loadReg(dst, $t0);
	if (offset->obj == ObjectiveType::constty) {
		if (base->typ == Type::chartp) {		// char
			int _offset = offset->addr + base->addr;	// base->addr = 0 for global var
			if (base->level == 1)
				add(MipsCode::lb, reg0, int2string(_offset), $sp);
			else 
				add(MipsCode::lb, reg0, base->ident_name + "+" + int2string(_offset));
		}
		else if (base->typ == Type::inttp) {	// int
			int _offset = offset->addr * 4 + base->addr;	// base->addr = 0 for global var
			if (base->level == 1)
				add(MipsCode::lw, reg0, int2string(_offset), $sp);
			else 
				add(MipsCode::lw, reg0, base->ident_name + "+" + int2string(_offset));
		}
		else 
			fatal("arrayLoad", "error type", base);
	}
	else if (offset->obj == ObjectiveType::varty || offset->obj == ObjectiveType::tmp) {	// int sll 2, 
		string reg1 = loadReg(offset, $t1);
		if (base->typ == Type::chartp) {
			if (base->level == 1) {
				// addr = $sp + base + CHARSIZE * offset
				add(MipsCode::add, $t1, $sp, reg1);
				add(MipsCode::lb, reg0, int2string(base->addr), $t1);
			}
			else
				add(MipsCode::lb, reg0, base->ident_name + "(" + reg1 + ")");
		}
		else if (base->typ == Type::inttp) {
			add(MipsCode::sll, $t1, reg1, "2");	// $t1 = offset * INTSIZE
			if (base->level == 1) {
				// addr = $sp + base + INTSIZE * offset
				add(MipsCode::add, $t1, $sp, $t1);	// $t1 = $sp + INTSIZE * offset
				add(MipsCode::lw, reg0, int2string(base->addr), $t1);
			}
			else
				// addr = varName($offset_reg) = name($t1);
				add(MipsCode::lw, reg0, base->ident_name + "($t1)");
		}
		else 
			fatal("arrayLoad", "type error", base);
	}
	else 
		fatal("arrayLoad", "obj error: expect const, var, tmp", offset);

	storeMem(reg0, dst);	// record saved status
	// global variable must storeMem
}

void mipsObjCode::arrayStore(SymbolItem* value, SymbolItem* offset, SymbolItem* base) {
	/* base[off] <= value
	*/
	string reg0 = loadReg(value, $t0);
	if (offset->obj == ObjectiveType::constty) {
		if (base->typ == Type::chartp) {
			int _offset = base->addr + offset->addr;
			if (base->level == 1)	// local var
				add(MipsCode::sb, reg0, int2string(_offset), $sp);
			else 					// global var
				add(MipsCode::sb, reg0, base->ident_name + "+" + int2string(_offset));
		}
		else if (base->typ == Type::inttp) {
			int _offset = base->addr + offset->addr * 4;
			if (base->level == 1)	// local var
				add(MipsCode::sw, reg0, int2string(_offset), $sp);
			else 	// global var
				add(MipsCode::sw, reg0, base->ident_name + "+" + int2string(_offset));
		}
		else {
			fatal("arrayStore", "type error: not int or char", base);
		}
	}
	else if(offset->obj == ObjectiveType::varty || offset->obj == ObjectiveType::tmp) {
		string reg1 = loadReg(offset, $t1);
		if (base->typ == Type::chartp) {	// $t1 = offset
			if (base->level == 1) {
				add(MipsCode::add, $t1, reg1, $sp);
				add(MipsCode::sb, reg0, int2string(base->addr), $t1);
			}
			else {
				add(MipsCode::sb, reg0, base->ident_name + "(" + reg1 + ")");
			}
		}
		else if (base->typ == Type::inttp) {	// $t1 = offset
			add(MipsCode::sll, $t1, reg1, "2");
			if (base->level == 1) {
				add(MipsCode::add, $t1, $t1, $sp);
				add(MipsCode::sw, reg0, int2string(base->addr), $t1);
			}
			else
				add(MipsCode::sw, reg0, base->ident_name + "($t1)");
		}
		else {
			fatal("arrayStore", "type error: not int or char", base);
		}
	}
	else {
		fatal("arrayStore", "obj error: expect const, var, tmp", offset);
	}
}

void mipsObjCode::read(SymbolItem* item) {
	add(MipsCode::note, "read");
	if (item->typ == Type::chartp) {
		add(MipsCode::li, $v0, "12");
	}
	else if (item->typ == Type::inttp) {
		add(MipsCode::li, $v0, "5");
	}
	else {
		fatal("read", "type error: not char nor int", item);
	}
	add(MipsCode::syscall);
	storeMem($v0, item);
}

void mipsObjCode::print(SymbolItem* str, SymbolItem* exp) {
	add(MipsCode::note, "print");
	if (str == NULL && exp == NULL) {
		add(MipsCode::li, $v0, "4");
		add(MipsCode::la, $a0, "_str_endl");
		add(MipsCode::syscall);
		return;
	}
	string sname;
	string strContent;
	stringstream ss;
	if (str != NULL) { // string exist
		ss << str->ident_name;
	}
	if (exp != NULL && exp->obj == ObjectiveType::constty) {
		if (exp->typ == Type::chartp)
			ss << (char)exp->addr;
		else if (exp->typ == Type::inttp)
			ss << exp->addr;
		else
			fatal("print", "type error: not char or int", exp);
	}
	if (!ss.str().empty()) {
		// we have read a string 
		if (exp == NULL || exp->obj == ObjectiveType::constty)
			ss << "\\n";
		strContent = ss.str();
		if (strTab.find(strContent) == strTab.end()) {
			// no same string
			ss.str("");
			ss << "_str_" << string_count++;
			sname = ss.str();
			strTab[strContent] = sname;
			ss.str("");
			ss << sname << ": .asciiz \"" << strContent << "\"";
			data.push_back("\t" + ss.str());
		}
		else {
			sname = strTab[strContent];
		}

		add(MipsCode::li, $v0, "4");
		add(MipsCode::la, $a0, sname);
		add(MipsCode::syscall);
	}
	if (exp && exp->obj != ObjectiveType::constty) {
		if (exp->typ == Type::chartp) {
			add(MipsCode::li, $v0, "11");
			if (exp->level == 0)
				add(MipsCode::lb, $a0, exp->ident_name);
			else {
				if (exp->isUsingReg && exp->allocated_reg != "") {
					add(MipsCode::move, $a0, exp->allocated_reg);
				}
				else
					add(MipsCode::lb, $a0, int2string(exp->addr), $sp);
			}
			add(MipsCode::syscall);
		}
		else if (exp->typ == Type::inttp) {
			add(MipsCode::li, $v0, "1");
			if (exp->level == 0)
				add(MipsCode::lw, $a0, exp->ident_name);
			else {
				if (exp->allocated_reg != "" && exp->isUsingReg)
					add(MipsCode::move, $a0, exp->allocated_reg);
				else
					add(MipsCode::lw, $a0, int2string(exp->addr), $sp);
			}
			add(MipsCode::syscall);
		}
		else
			fatal("print", "type error", exp);
		add(MipsCode::li, $v0, "4");
		add(MipsCode::la, $a0, "_str_endl");
		add(MipsCode::syscall);
	}
/*
	if (str == NULL && exp == NULL) {
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
		ss << ": .ascii \"";
		ss << item->ident_name;
		ss << "\"";
		data.push_back("\t" + ss.str());
		add(MipsCode::li, $v0, "4");
		add(MipsCode::la, $a0, sname);
		add(MipsCode::syscall);
	}
	else if (item->typ == Type::chartp) {
		add(MipsCode::li, $v0, int2string(11));
		if (item->obj == ObjectiveType::constty) {
			add(MipsCode::li, $a0, int2string(item->addr));
		}
		else{	// li,
			if (item->level == 0)
				add(MipsCode::lb, $a0, item->ident_name);
			else
				add(MipsCode::lb, $a0, int2string(item->addr), $sp);
		}
		add(MipsCode::syscall);
	}
	else if (item->typ == Type::inttp) {
		add(MipsCode::li, $v0, int2string(1));
		if (item->obj == ObjectiveType::constty) {
			add(MipsCode::li, $a0, int2string(item->addr));
		}
		else{	// integer variable
			if (item->level == 0)
				add(MipsCode::lw, $a0, item->ident_name);
			else
				add(MipsCode::lw, $a0, int2string(item->addr), $sp);
		}
		add(MipsCode::syscall);
	}
	else {
		fatal("print", "type error", item);
	}
*/
}




