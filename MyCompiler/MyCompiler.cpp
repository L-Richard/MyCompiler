// MyCompiler.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include "Token.h"
#include "Const.h"
#include "Lexer.h"
#include "Error.h"
using namespace std;
string filename = "15071111_test_lexer.txt";
int main()
{
	ofstream outFile;
	outFile.open("result.txt");
	Error error_handler = Error();
	Lexer *lexer = new Lexer(filename, error_handler);
	while (true) {
		Token t = lexer->getSym();
		if (t.getcc() == -1) {
			break;
		}
		cout << "  column: " << t.getcc() << ",  rows: " << t.getlc()
			 << ",\ttype: " << Enum2Str(t.getSymbol()) << " \"" << t.getIdentName() << "\", ";
		switch (t.getType()) {
		case Symbol::ident:		cout << "ident name : " << t.getIdentName() << endl;	break;
		case Symbol::stringcon: cout << "string const: " << t.getIdentName() << endl; break;
		case Symbol::intcon:	cout << "integer const: " << t.getConstNum() << endl; break;
		case Symbol::charcon:	cout << "charactor const: " << t.getIdentName() << endl; break;
		default:				cout << endl; break; 
		}
	}
}

