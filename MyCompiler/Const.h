#pragma once
#include <set>
#include "Token.h"
typedef set<Symbol> SymSet;
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
const SymSet statementBsys = {
	Symbol::lbrace,
	Symbol::ifsy,
	Symbol::switchsy,
	Symbol::ident,
	Symbol::whilesy,
	Symbol::printfsy,
	Symbol::scanfsy,
};
const SymSet compoundBsys = {
	Symbol::intsy,
	Symbol::charsy,
	Symbol::lbrace,
	Symbol::ifsy,
	Symbol::switchsy,
	Symbol::ident,
	Symbol::whilesy,
};
const SymSet factorBsys = {
	Symbol::intcon,
	Symbol::charcon,
	Symbol::ident,
	Symbol::lparent
};

const int nkw = 27;  
const int alng = 10; 
const int llng = 1024;			// max line length 
const int emax = 322;
const int emin = -292;
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
