#pragma once
#include <string>
#include <map>

// Token�м�¼�����ݣ�
using namespace std;
enum Symbol {
	notsy,ident, 
	intcon, charcon, stringcon,		// ����
	plus, minus, times, slash,		// �����
	eql, neq, gtr, geq, les, leq,	// �߼������
	// ����
	lparent, rparent,
	lsquare, rsquare,
	lbrace, rbrace,
	// ������
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
	Symbol sym;
	string ident_name;	// name, stringcon, charcon
	int lc, cc, inum;
public:
	Token(Symbol sym, int lc, int cc, string ident_name="", int inum=0);
	int getlc();
	int getcc();
	int getConstNum();
	string getIdentName();
	string getString();
	Symbol getSymbol();
	char getConstChar();

	bool isEnd();
	~Token();
};

