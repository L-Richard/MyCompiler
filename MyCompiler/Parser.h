#pragma once
#include "Lexer.h"
#include "MidCodeGen.h"
#include "SymbolTable.h"
#include "Const.h"

class Parser {
private:
	Lexer* lexer;
	Error& error_handler;
	MidCodeGen& codeGen;
	SymbolTable& symTab;

	Token current_token;	// record a lexem
	Token type_token;		// record type of a var or const or function
	Token name_token;
	bool hasRet = false;	// if a function defination has return value
	bool returned = false;	// check last function statement is return
	SymbolItem* fun_label = NULL;

	void skip(SymSet fsys);		// skip words until current_token's type in fsys,
	void test(SymSet s1, SymSet s2, int no);	// 
	void testSemicolon();
	void nextSym();
	Type tokenType();
	
	void constDefinition();
	void varDeclaration();
	int  signedInt(SymSet fsys);
	SymbolItem* selector(SymSet fsys);		//* return the index: [exp], 
	void paraList();
	SymbolItem* function();
	void statement();
	void condition(SymbolItem* label);
	SymbolItem* expression(SymSet fsys);
	SymbolItem* item(SymSet fsys);
	SymbolItem* factor(SymSet fsys);
	void ifStatement(SymSet fsys);
	void switchStatement();
	/* switch -> if else */
	void caseStatement(SymbolItem* expItem, SymbolItem* end_switch, map<int, Type> *caseTab);
	/* return const value, and check duplication
	SymbolItem *expItem, *endSwitchLabel, */
	void defaultStatement();
	// just do it, no jmp
	void whileStatement();
	SymbolItem* call(SymSet fsys, SymbolItem* funItem);
	// funItem is self SymbolItem
	void returnStatement();
	SymbolItem* assignStatement();
	void multiStatement();
	void compoundstatement();
	void read();
	void write();

public:
	void parse();
	Parser(Lexer* lexer, Error& error_handler, MidCodeGen& codeGen, SymbolTable &symTab);
	~Parser();
};

