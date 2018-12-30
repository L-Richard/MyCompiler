#pragma once
#include "Lexer.h"
#include "MidCodeGen.h"
#include "SymbolTable.h"
#include "Const.h"
#include <sstream>

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
	bool isMain = false;
	SymbolItem* mainLabel = codeGen.genLabel(labelType::main_lb);
	SymbolItem* endMainLabel = codeGen.genLabel(labelType::end_main_lb);

	void skip(SymSet fsys);		// skip words until current_token's type in fsys,
	void test(SymSet s1, SymSet s2, int no);	// 
	void testSemicolon();
	void nextSym();
	Type tokenType();
	
	void constDefinition();
	void varDeclaration();
	int  signedInt(SymSet fsys);
	SymbolItem* selector(SymbolItem* item,SymSet fsys);		//* return the index: [exp], 
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

	int computeConditionVal(int a, int b, Symbol opSym) {
		/* compute condition result
		return 1 if true, 0 if false, -1 if error.
		*/
		if (opSym == Symbol::leq) {
			return a <= b;
		}
		else if (opSym == Symbol::les) {
			return a < b;
		}
		else if (opSym == Symbol::eql) {
			return a == b;
		}
		else if (opSym == Symbol::neq) {
			return a != b;
		}
		else if (opSym == Symbol::gtr) {
			return a > b;
		}
		else if (opSym == Symbol::geq) {
			return a >= b;
		}
		else
			return -1;
	}
	string int2str(int i) {
		stringstream ss;
		ss << i;
		return ss.str();
	}
public:
	void parse();
	Parser(Lexer* lexer, Error& error_handler, MidCodeGen& codeGen, SymbolTable &symTab);
	~Parser();
};

