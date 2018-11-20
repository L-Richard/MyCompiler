#include "pch.h"
#include "Parser.h"

/*
Parser::Parser(Lexer& lexer, Error& error_handler, MidCodeGen& codeGen, SymbolTable& symTab)
:lexer(lexer),error_handler(error_handler), codeGen(codeGen), symTab(symTab)
 {
}
*/


Parser::~Parser()
{
}

void Parser::nextSym() {
	current_token = lexer.getSym();
}

void Parser::skip(SymSet fsys) {
	while (!fsys.count(current_token.getType())) {
		nextSym();
	}
}

void Parser::test(SymSet s1, SymSet s2, int no) {
	if (!s1.count(current_token.getType())) {
		// **** error_handler;
		
		s1.insert(s2.begin(), s2.end());
		skip(s2);
	}
}

void Parser::enter(const Token& t, ObjectiveType obj, int level) {

}


void Parser::parse() {
	// parse the whole source file
	bool varDefDone = false;
	bool isMain = false;
	nextSym();
	// **** test();
	// const definition
	while (current_token.getType() == Symbol::constsy) {
		nextSym();
		constDefinition();
	}

	SymSet varFunDeclBegSym = { Symbol::intsy, Symbol::charsy, Symbol::voidsy };

	// **** test(), skip to varFunDeclBegSym
	// variable definition
	// function
	while (!current_token.isEnd() && varFunDeclBegSym.count(current_token.getType())) {
		if (current_token.getType() == Symbol::voidsy) {
			varDefDone = true;
		}
		type_token = current_token;		// record type of var or func
		nextSym();
		name_token = current_token;	// record name of var or func
		// check if redefine an identifier
		// **** searchTab(current_token);
		nextSym();
		if (current_token.getType == Symbol::lparent) {
			varDefDone = true;
		}
		if (type_token.getType() == Symbol::voidsy && name_token.getIdentName == "main") {
			// main function no parameters, no return value;

			break;
		}
		if (varDefDone) {
			if (type_token.getType == Symbol::voidsy) {
				funWithoutRet();
			}
			else {
				funWithRet();
			}
		}
		else {
			varDeclaration();
		}
	}
	// change the error no
	test({ Symbol::eofsy }, {}, 0);
}
int Parser::signedInt(SymSet fsys) {
	SymSet tmp = { Symbol::minus, Symbol::plus, Symbol::intcon };
	Symbol sy = current_token.getType();
	int sign = 1;
	test(tmp, fsys, 0);
	if (tmp.count(sy)) {
		if (sy == Symbol::minus) {
			sign = -1;
			nextSym();
		}
		else if (sy == Symbol::plus) {
			nextSym();
		}
		if (current_token.getType() == intcon) {
			return current_token.getConstNum * sign;
		}
		else {
			// error

			return 0;
		}
	}
}
void Parser::constDefinition() {
	Symbol sy;
	type_token = current_token;
	// error if not char or int
	do {
		nextSym();
		name_token = current_token;
		// check symbol table
		nextSym();
		// =
		if (current_token.getType() != Symbol::becomes) {
			// error_handler.reportErrorMsg();
		}
		else {
			nextSym();
		}
		
		if (type_token.getType() == Symbol::intsy) {
			int tmpConstInt = signedInt({});	// ?????????????????????? fsys
			// enter symbol table
		}
		else if (type_token.getType() == Symbol::charsy) {
			if (current_token.getType() == charcon) {
				// enter symbol table 
			}
			else {
				// error: type not coherent
				
			}
		}
		else {
			// error: not type
		}
		// skip to definition end
		// test();

		////////////////////////////
		/*
		sy = current_token.getType();
		if (sy == intcon || sy == charcon) {
			// token is a constant, check type
			if (type_token.getType() == intcon && sy == intcon) {
				int tmpContstInt = current_token.getConstNum();
				// enter symbol table

				nextSym();
			}
			else if (type_token.getType() == charcon && sy == charcon) {
				char tmpConstChar = current_token.getConstChar();
				// enter symbol table
				
				nextSym();
			}
			else {
				// type is not coherent
				// **** error_handler
			}
		}
		else {
			// not a const int or const char 
			// **** error_handler.reportErrorMsg();
		}
		*/
	} while (current_token.getType() == Symbol::comma);
	if (current_token.getType() == Symbol::semicolon) {
		nextSym();
	}
	// **** test; // check semicolon, 
	// skip to comma, ident, int, char, void,
#ifdef DEBUG_Gramma_analysis
	cout << "常量定义语句" << endl;
#endif
}

void Parser::varDeclaration() {
	/* before call this function, we use type_token to record 
	 the type of var and we use name_token to record var's name.
	 And the current_token is not a '(', it means not function 
	 declaration. If it is ',', we should declare multiple var.
	 */
	 // check the symbol table, error if redefine var
	bool firstLoop = true;
	do {
		if (!firstLoop) {
			nextSym(); // read an ident
		}
		firstLoop = false;

		if (current_token.getType() == Symbol::lsquare) {
			// array type
			nextSym();
			Symbol sy = current_token.getType();
			int tmpArrayMax = 0;
			if (sy == Symbol::plus || sy == Symbol::minus) {
				// error
#ifdef DEBUG_Gramma_analysis
				cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
				cout << "错误：数组索引必须是无符号整数" << endl;
#endif
				nextSym();
			}
			
			// read a num
			if (current_token.getType() == Symbol::intcon) {
				tmpArrayMax = current_token.getConstNum();
				nextSym();
				if (current_token.getType() == Symbol::rsquare) {
					// no problem and enter symbol table
					nextSym();
				}
				else {
#ifdef DEBUG_Gramma_analysis
					cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
					cout << "错误：缺少右中括号" << endl;
#endif // DEBUG_Gramma_analysis
					// skip();
				}
			}
			else {
#ifdef DEBUG_Gramma_analysis
				cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
				cout << "错误：缺少无符号整数常量" << endl;
#endif // DEBUG_Gramma_analysis
				// skip();
			}
		}
		else {
			// comman variable, enter symbol table

			nextSym();
		}
	} while (current_token.getType() == Symbol::comma);
	// **** test(); 
	// must semicolon end the declar
#ifdef DEBUG_Gramma_analysis
	cout << "This is variable declaration statement!" << endl;
#endif // DEBUG_Gramma_analysis
}

void Parser::funWithoutRet() {
	// test lbrace,

}

void Parser::compoundstatement() {
	// while sy in statement begin symbol set
}
void Parser::block() {

}

void Parser::statement() {
	Symbol sy = current_token.getType();
	if (statementBsys.count(sy)) {
		switch (sy) {
		case Symbol::lbrace:	block();			break;
		case Symbol::ifsy:		ifStatement({});		break;
		case Symbol::switchsy:	switchStatement();	break;
		case Symbol::whilesy:	whileStatement();	break;
		case Symbol::scanfsy:	read();				break;
		case Symbol::printfsy:	write();			break;
		case Symbol::ident: {
			// search symbol table
			nextSym();
			// ？？？？？？？？？？？？？？？？？？？？？？需要改进
			switch (current_token.getType()) {
			case Symbol::becomes:	assignStatement();	break;
			case Symbol::lparent:	call();				break;	
			default: {
#ifdef DEBUG_Gramma_analysis
				cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
				cout << "出错：标识符后需要等号或小括号！" << endl;
#endif // DEBUG_Gramma_analysis
				
			}
			}
			break;
		}
		default: {
#ifdef DEBUG_Gramma_analysis
			cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
			cout << "出错：不是语句开头符号!" << endl;
#endif // DEBUG_Gramma_analysis
		}
		}
	}
}

void Parser::ifStatement(SymSet fsys) {
	// ？？？？？？？？？？？？？？？？？？？？？？？？？error id
	SymSet fsys;
	fsys.insert(statementBsys.begin(), statementBsys.end());
	fsys.insert({ Symbol::lbrace });
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	cout << lc << "行: ";
	cout << "if语句" << endl;
#endif // DEBUG_Gramma_analysis
	nextSym();
	test({ Symbol::lparent }, statementBsys, 0);
	if (current_token.getType() == Symbol::lparent) {
		nextSym();
		condition();
		if (current_token.getType() == Symbol::rparent) {
			nextSym();
		}
		else {
			// error: missing rparent
#ifdef DEBUG_Gramma_analysis
			cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
			cout << "出错：不是语句开头符号!" << endl;
#endif // DEBUG_Gramma_analysis
		}
		test({ Symbol::lbrace }, fsys, 0);

		if (current_token.getType() == lbrace) {
			multiStatement();
			test({ Symbol::rbrace }, statementBsys, 0);
		}
	}
}

void Parser::switchStatement() {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	cout << lc << "行: ";
	cout << "switch语句" << endl;
#endif // DEBUG_Gramma_analysis
	nextSym();
	test({ Symbol::lparent }, { statementBsys }, 0);
	if (current_token.getType() == lparent) {
		nextSym();
		expression();
		if (current_token.getType() == rparent) {
			nextSym();
		}
		else {
			//error
#ifdef DEBUG_Gramma_analysis
			cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
			cout << "出错：缺少右括号" << endl;
#endif // DEBUG_Gramma_analysis
		}
	// 	SymSet tmp = statementBsys;
	//	test({ lbrace }, {})
	}
}

void Parser::caseStatement() {
	nextSym();
	// test expression ??
	expression();
	if (current_token.getType() == Symbol::colon) {
		nextSym();
	}
	else {
#ifdef DEBUG_Gramma_analysis
			cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
			cout << "出错：缺少冒号" << endl;
#endif // DEBUG_Gramma_analysis
	}
	// test colon
	statement();
}

void Parser::expression() {
	if (current_token.getType() == Symbol::plus
		|| current_token.getType() == Symbol::minus) {
		// terms are connected with + or -
	}

}

void Parser::item(SymSet fsys) {

	SymSet tmpFacFsys = fsys;
	tmpFacFsys.insert({Symbol::times, Symbol::slash});
	factor(tmpFacFsys);
	while (current_token.getType() == Symbol::times
		|| current_token.getType() == Symbol::slash) {
		// factor is connected with * or /
		// need to record the operator 
		nextSym();
		factor(tmpFacFsys);
	}
}

void Parser::factor(SymSet fsys) {
	test(factorBsys, fsys, 0);
	if (factorBsys.count(current_token.getType())) {
		switch (current_token.getType()) {
		case Symbol::ident: {
			// search symbol table
			nextSym();
			switch (current_token.getType()) {
			case Symbol::lparent:	call();		break;
			case Symbol::lsquare:	selector();	break;
			default: {
				// load the value of this identifier
				
			}
			}
			break;
		}
		case Symbol::intcon: {
			// do something
			nextSym();
			break;
		}
		case Symbol::charcon: {
			// do something
			nextSym();
			break;
		}
		case Symbol::lparent: {
			nextSym();
			expression();
			test({ rparent }, fsys, 0);
			break;
		}

		}
	}
}

