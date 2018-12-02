#pragma once
#include <list>
#include <string>
#include "Const.h"
#include "Error.h"
enum ObjectiveType {
	noty,
	constty,	// defined const, with a name
	varty,
	functy,
	arrayty,
	tmp,		// tmp
	constValue,	// const in expression
	label,
};
enum Type {
	notp,
	chartp,
	inttp,
	stringtp,
	voidtp,
};
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
	// expItem, DAG graph
	bool isConst = false;
	SymbolItem* exp_left = NULL;
	SymbolItem* exp_right = NULL;
	// function 
	map<string, SymbolItem*> *link = NULL;	// function tab
	list<Type> paraList;
	bool hasRet = true;			// 
	int paraSize = 0;			// 
	int localSize = 0;
	int level = 0;
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
	int stack = 0;	// record global variable space
	int level = 0;	// 0: global ident. 1: local ident
public:
	void enterFunc(Token name_token, enum Type typ);
	void enterVar(Token name_token, enum Type typ, int arraySize);
	void enterPara(Token name_token, enum Type typ);
	void enterConst(Token name_token, enum Type typ, int constValue);
	SymbolItem* search(Token name);
	void funDone();
	SymbolTable(Error&error_handler);
	~SymbolTable();
};

