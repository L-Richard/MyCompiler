#pragma once
#include <set>
#include "Token.h"

typedef set<Symbol> SymSet;
const SymSet typeSys = {
	intsy, 
	charsy,
	voidsy,
};
const SymSet blockBsys = { 
	Symbol::lbrace, 
	Symbol::ident 
};
const SymSet constDefFsys = {
	Symbol::constsy,
	Symbol::intsy,
	Symbol::charsy,
	Symbol::voidsy,
};
const SymSet globalVarDecFsys = {
	Symbol::intsy,
	Symbol::charsy,
	Symbol::voidsy,
};
const SymSet conValueBsys = {
	Symbol::intcon,
	Symbol::charcon,
	Symbol::plus,
	Symbol::minus,
};
const SymSet statementBsys = {
	Symbol::lbrace,
	Symbol::ifsy,
	Symbol::switchsy,
	// Symbol::casesy,	// case后面可能是空语句，可能执行下个case
	Symbol::whilesy,
	Symbol::scanfsy,
	Symbol::printfsy,
	Symbol::returnsy,
	Symbol::semicolon,
	Symbol::ident,
};
const SymSet compoundBsys = {
	Symbol::intsy,
	Symbol::charsy,
	Symbol::lbrace,
	Symbol::ifsy,
	Symbol::switchsy,
	// Symbol::casesy,
	Symbol::whilesy,
	Symbol::printfsy,
	Symbol::scanfsy,
	Symbol::returnsy,
	Symbol::semicolon,
	Symbol::ident,
};
const SymSet logicalOperatorSys = {
	// 二元逻辑运算符集合
	Symbol::eql, 
	Symbol::neq, 
	Symbol::gtr, 
	Symbol::geq, 
	Symbol::les, 
	Symbol::leq,	
};
const SymSet expBsys = {
	Symbol::plus,
	Symbol::minus,
	Symbol::ident,
	Symbol::lparent,
	Symbol::intcon,
	Symbol::charcon,
};

const set<string> regSet = {
	"$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
	"$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "$t9",

};

const int INTSIZE = 4;
const int CHARSIZE = 1;
const int nkw = 27;  
const int alng = 10; 
const int llng = 1024;	// max line length 
const int ilmax = 10;	// integer length max
const int tmax = 100; 
const int bmax = 20; 
const int amax = 30; 
const int c2max = 20; 
const int csmax = 30; 
const int cmax = 800; 
const int lmax = 7; 
const int smax = 600; 
const int ermax = 58; 
const int omax = 63; 
const int xmax = 32767; 
const int nmax = 32767; 
const int lineleng = 132; 
const int linelimit = 200; 
const int stacksize = 1450;
