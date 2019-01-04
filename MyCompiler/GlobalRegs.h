#pragma once
#include "SymbolTable.h"
#include "MidCodeGen.h"
#include <algorithm>
#include <iterator>

struct ConflictNode {
	SymbolItem* var = NULL;
	set<ConflictNode*> link;
	bool isAllocatable = true;

	ConflictNode(SymbolItem * var) {
		this->var = var;
	}
};

struct Base {
	// Quadruples:
	list<Quadruples*> baseCodes;

	set<SymbolItem*> defVars;
	set<SymbolItem*> useVars;
	set<SymbolItem*> inVars;
	set<SymbolItem*> outVars;
	set<SymbolItem*> labels;
	set<Base*> lastBases;
	set<Base*> nextBases;
	SymbolItem* gotoLabel = NULL;
	bool isChanged = false;
	bool isBase = false;		// base or separator

	// allocated registers 
	set<string> regs_banned;

	// global vars = in + out.
	// base vars = def + use.

	// function:
	void inUse(SymbolItem* var) {
		if (var != NULL 
			&& var->level != 0		// ȫ�ֱ���������Ĵ���
			&& !this->defVars.count(var) 
			&& !notRecordSet.count(var->obj)) {
			
			this->useVars.insert(var);
		}
	}
	void inDef(SymbolItem* var) {
		if (var != NULL 
			&& var->level != 0		// ȫ�ֱ���������Ĵ���
			&& !this->useVars.count(var)
			&& !notRecordSet.count(var->obj)) {

			this->defVars.insert(var);
		}
	}
	const set<ObjectiveType> notRecordSet = {
		ObjectiveType::arrayty,
		ObjectiveType::functy,
		ObjectiveType::constty,
		ObjectiveType::label,
	};
};


class GlobalRegs {
	SymbolTable &tab;
	vector<Quadruples> &midCodes;

	SymbolItem* funItem = NULL;
	// regs:
	const set<string> allRegs = {
		"$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
		"$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "$t9",
		"$a1", "$a2", "$a3",
	};

	void baseBlock(vector<Quadruples>::iterator funHead, vector<Quadruples>::iterator funTail);		// ���ֻ�����
	void dsa(list<Base*> &baseList);				// ����������
	void conflictGraph(list<Base*> &baseList, map<SymbolItem*, ConflictNode*> &hashTab);	// ������ͻͼ

	// ����Ĵ���
	void graphColor(map<SymbolItem*, ConflictNode*> &hashTab);
	void allocateReg(ConflictNode* node);

	// ������Ĵ�������
	void baseAllocateReg(list<Base*> &baseList);
	void varCodeRecord(SymbolItem* var, Quadruples* code, map<SymbolItem*, Quadruples*> &var_code) {
		// ֻ��¼�������ھֲ�����
		if (var != NULL)
			var_code[var] = code;
	}

	// some ...
	bool isFunHead(Quadruples* code) {
		if (code->op == Operator::setLabel
			&& (code->dst->lab == labelType::fun_lb || code->dst->lab == labelType::main_lb)) {
			return true;
		}
		return false;
	}

public:
	void allocateRegisters();
	GlobalRegs(SymbolTable &tab, vector<Quadruples> &midCodes);
	~GlobalRegs();
};

