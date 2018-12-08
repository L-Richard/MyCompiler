#pragma once
#include <list>
#include <string>
#include "Const.h"
#include "Error.h"
#include "c0_enum.h"
/*
struct bTabItem {
	std::string ident_name = "";
	enum ObjectiveType obj = noty;
	enum Symbol typ = notsy;
	int addr = 0;	// varibale: offset from base of this block
					// constant: char ascii or sigend integer
};
*/


struct SymbolItem {
	std::string ident_name = "";		// global ident
	enum ObjectiveType obj = noty;
	enum Type typ = notp;
	int addr = 0;	// varibale: offset from base of this block
					// constant: char ascii or sigend integer
	// array
	int arrayNumMax = 0;
	int varSize = 0;
	// expItem, DAG graph
	bool isConst = false;
	SymbolItem* exp_left = NULL;
	SymbolItem* exp_right = NULL;
	// function 
	map<string, SymbolItem*> *link = NULL;	// function tab
	SymbolItem* fun_label = NULL;
	list<Type> paraList;
	list<SymbolItem*> paras;
	bool hasRet = false;			// 
	int level = 0;
	// label
	labelType lab;

	int totalSize = 0;		// totalSize = paraSize + tmpSize
	int paraSize = 0;		// 
	int tmpSize = 0;
};



class SymbolTable {
private:
	Error &error_handler;
	map<string, SymbolItem*> symTab;
	map<string, SymbolItem*> *ftab = NULL;
	SymbolItem *lastFunItem = NULL;	// update paraSize and localSize

	/* stack:
		first, record global variable space
		then,  record parameters and local variable space
	*/
	int stack = 4;	// 4: $ra. record variable space in function
	int level = 0;	// 0: global ident. 1: local ident
public:
	void enterFunc(Token name_token, enum Type typ, SymbolItem* label);
	void enterVar(Token name_token, enum Type typ, int arraySize);
	void enterPara(Token name_token, enum Type typ);
	void enterConst(Token name_token, enum Type typ, int constValue);
	void enterTemp(SymbolItem* tmp);
	SymbolItem* search(Token name);
	void funDone();

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

	SymbolItem* getFunItem() {
		return lastFunItem;
	}
	map<string, SymbolItem*>& getTab() {
		return symTab;
	}

	SymbolTable(Error&error_handler);
	~SymbolTable();
};

