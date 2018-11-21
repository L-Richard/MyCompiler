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
//	SymbolTable& symTab;
	Token current_token;		// record a lexem
	Token type_token;		// record type of a var or const or function
	Token name_token;

	// symbol table: 
	vector<SymbolItem> symTab;

	// function: need btab, return flag
	bool hasRet;

	// need to record an identifier's Symbol type, adr,  

	class item {
		// record expression msg

	};
	void skip(SymSet fsys);		// skip words until current_token's type in fsys,
	void test(SymSet s1, SymSet s2, int no);	// 
	void testSemicolon();
	void enter(const Token& t, ObjectiveType obj, bool isGlobal);
	// search table for a 
	int searchTab(const Token& identifier);	
	void nextSym();
	
	void constDefinition();
	void varDeclaration();
	int  signedInt(SymSet fsys);
	void selector();
	void paraList();
	void function();
	void statement();
	void condition();
	void expression();
	void item(SymSet fsys);
	void factor(SymSet fsys);
	void ifStatement(SymSet fsys);
	void switchStatement();
	void caseStatement();
	void defaultStatement();
	void whileStatement();
	void call();
	void returnStatement();
	void assignStatement();
	void multiStatement();
	void compoundstatement();
	void read();
	void write();

public:
	void parse();
	Parser(Lexer* lexer, Error& error_handler, MidCodeGen& codeGen);
//, SymbolTable& symTab);
//	Parser(Lexer &lexer, Error &error_handler, MidCodeGen &codeGen, SymbolTable &symTab);
	~Parser();
};

