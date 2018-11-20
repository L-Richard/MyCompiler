#pragma once
#include <string>
#include <map>

// Token中记录的内容：
using namespace std;
enum Symbol {
	notsy,ident, 
	intcon, charcon, stringcon,		// 常量
	plus, minus, times, slash,		// 运算符
	eql, neq, gtr, geq, les, leq,	// 逻辑运算符
	// 括号
	lparent, rparent,
	lsquare, rsquare,
	lbrace, rbrace,
	// 标点符号
	comma,		// ,
	semicolon,	// ;
	period,		// .
	colon,		// :
	quote,		// '
	dquote,		// "
	becomes, 	// =	
	// key word
	ifsy, switchsy, casesy, whilesy,
	intsy, constsy, charsy, voidsy,
	printfsy, scanfsy,
	// end of file
	eofsy
};
string Enum2Str(Symbol sym);
class Token {
private:
	Symbol type;
	string ident_name;
	int lc, cc, inum;
public:
	Token(Symbol type, int lc, int cc, string ident_name="", int inum=0);
	int getlc();
	int getcc();
	int getConstNum();
	string getIdentName();
	string getString();
	Symbol getType();
	char getConstChar();

	bool isEnd();
	~Token();
};

