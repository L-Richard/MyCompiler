#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include "Token.h"
#include <map>
#include "Const.h"
#include "Error.h"

using namespace std;

class Lexer {
	ifstream source_file;
	char ch = ' ';
	int ll = 0;			// line length
	int lc = 0;			// line counter
	int cc = 0;			// char counter
	char line[llng];	// line
	map<string, Symbol> reserved_words;

	string ident_name;
	Symbol sym;
	int inum;
	
	Error& error_handler;

	void initial_reserved_word();
	void getch();
public:
	Lexer(string filename, Error& error_handler);
	Token getSym();
	~Lexer();
};
