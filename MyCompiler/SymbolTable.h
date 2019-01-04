#pragma once
#include <list>
#include <string>
#include "Const.h"
#include "Error.h"
#include "c0_enum.h"

struct SymbolItem {
	std::string ident_name = "";	// global ident
	string allocated_reg = "";		// 分配得到的寄存器
	bool isUsingReg = false;	// 记录是否正在使用reg，在生成目标代码中使用
	bool saved = true;

	enum ObjectiveType obj = noty;
	enum Type typ = notp;
	int addr = 0;	// varibale: offset from base of this block
					// constant: char ascii or sigend integer
	// array
	int arrayNumMax = 0;
	int varSize = 0;
	
	// function item
	map<string, SymbolItem*> *link = NULL;	// function tab
	SymbolItem* fun_label = NULL;
	list<Type> paraList;
	vector<SymbolItem*> paras;		// 用于传递参数
	bool hasRet = false;			// 
	int level = 0;

	// function symbol item
	int totalSize = 0;		// totalSize = paraSize + tmpSize
	int paraSize = 0;		// 
	int tmpSize = 0;
	vector<SymbolItem*> paraItems;	// 用于函数记录自身参数
	// int paraNum = 0;

	// label
	labelType lab;
	SymbolItem* funItem = NULL;	// fun_lb -> fun

	// Optimize: regs
	bool initial = true;		// true:没有初始值. 在生成目标码和符号表填参数时更改
	bool isGlobal = false;		// 跨基本块变量

	bool isPara = false;
	// string paraReg = "";		// 参数得到的寄存器
	// string current_reg = "";		// if "", in mem or initial.

	bool hasReg = false;		// 在优化分配寄存器使用
};


class SymbolTable {
private:
	Error &error_handler;
	map<string, SymbolItem*> symTab;
	map<string, SymbolItem*> *ftab = NULL;
	SymbolItem *lastFunItem = NULL;	// update paraSize and localSize
	
	const vector<string> paraRegs = { "$a1", "$a2", "$a3", "$k0", "$k1" };
	/* stack:
		first, record global variable space
		then,  record parameters and local variable space
	*/
	int stack = 4;	// 4: $ra. record variable space in function
	int level = 0;	// 0: global ident. 1: local ident


	// private method:
	void align() {
		int r = stack % 4;
		int q = stack / 4;
		if (r != 0) {
			stack += 4 - r;
		}
	}
	int typeSize(Type tp) {
		switch (tp) {
		case Type::chartp:	return CHARSIZE;
		case Type::inttp:	return INTSIZE;
		default:			return 0;
		}
	}


public:
	void enterFunc(Token name_token, enum Type typ, SymbolItem* label);
	SymbolItem* enterVar(Token name_token, enum Type typ, int arraySize, bool isInitial=true);
	void enterPara(Token name_token, enum Type typ);
	void enterConst(Token name_token, enum Type typ, int constValue);
	void enterTemp(SymbolItem* tmp);
	SymbolItem* search(Token name);
	void funDone();

	// get method
	SymbolItem* getFunItem() {
		return lastFunItem;
	}
	map<string, SymbolItem*>& getTab() {
		return symTab;
	}

/************************* Optimization  ****************************/
	void removeLastTemp(SymbolItem* Temp);
	// void killVar(SymbolItem* var);	
	// set some var not active, and will never be uesd

	SymbolTable(Error&error_handler);
	~SymbolTable();
};

