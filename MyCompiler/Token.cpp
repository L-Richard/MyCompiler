#include "pch.h"
#include "Token.h"
#include <iostream>

Token::Token(Symbol sym, int lc, int cc, string ident_name, int inum) {
	this->sym = sym;
	this->ident_name = ident_name;
	this->cc = cc;
	this->lc = lc;
	this->inum = inum;
}
bool Token::isEnd() {
	return sym == Symbol::eofsy;
}
int Token::getlc() {
	return this->lc;
}
int Token::getcc() {
	return this->cc;
}

Symbol Token::getSymbol() {
	return this->sym;
}

char Token::getConstChar() {
	if (this->sym == Symbol::charcon) {
		return this->ident_name[0];
	}
	else {
#ifdef DEBUG_TOKEN_MSG
		cout << "Error in Token::getConstChar: this token is not const char sym!" << endl;
#endif
		return 0;
	}
}

int Token::getConstNum() {
	if (this->sym == Symbol::intcon)
		return this->inum;
	else {
#ifdef DEBUG_TOKEN_MSG
		cout << "Error in Token:: getConstNum: this token is not a const int!" << endl;
#endif // DEBUG_TOKEN_MSG
		return 0;
	}
}

string Token::getIdentName() {
#ifdef DEBUG_TOKEN_MSG
	if (!this->sym == Symbol::ident) 
		cout << "Error in Token::getIdentName: this token is not an ident sym!" << endl;
#endif
	return this->ident_name;
}

string Token::getString() {
	//  record string const and char const with first char 
	if (this->sym == Symbol::stringcon)
		return this->ident_name;
	else {
#ifdef DEBUG_TOKEN_MSG
		cout << "Error in Token::getString: this token is not const string sym!" << endl;
#endif
		return "";
	}
}



Token::~Token()
{
}
string Enum2Str(Symbol sym) {
	switch (sym) {
	case notsy: return "notsy";
	case ident: return "ident";
	case intcon: return "intcon";
	case charcon: return "charcon";
	case stringcon: return "stringcon";
	case Symbol::plus: return "plus";
	case Symbol::minus: return "minus";
	case Symbol::times: return "times";
	case Symbol::slash: return "slash";
	case Symbol::eql: return "eql";
	case Symbol::neq: return "neq";
	case Symbol::gtr: return "gtr";
	case Symbol::geq: return "geq";
	case Symbol::les: return "les";
	case Symbol::leq: return "leq";
	case Symbol::lparent: return "lparent";
	case Symbol::rparent: return "rparent";
	case Symbol::lsquare: return "lsquare";
	case Symbol::rsquare: return "rsquaer";
	case Symbol::lbrace:  return "lbrace";
	case Symbol::rbrace:  return "rbrace";
	case Symbol::comma: return "comma";
	case Symbol::semicolon: return "semicolon";
	case Symbol::period: return "period";
	case Symbol::colon: return "colon";
	case Symbol::quote: return "quote";
	case Symbol::dquote: return "dquote";
	case Symbol::becomes: return "becomes";
	case Symbol::ifsy: return "ifsy";
	case Symbol::switchsy: return "switchsy";
	case Symbol::casesy: return "casesy";
	case Symbol::whilesy: return "whilesy";
	case Symbol::intsy: return "intsy";
	case Symbol::constsy: return "constsy";
	case Symbol::charsy: return "charsy";
	case Symbol::voidsy: return "voidsy";
	
	case Symbol::printfsy: return "printfsy";
	case Symbol::scanfsy: return "scanfsy";
	case Symbol::eofsy: return "eofsy";
	default: return "sym not found";
	}
}