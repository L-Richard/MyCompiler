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
	while (!fsys.count(current_token.getSymbol())) {
		nextSym();
	}
}

void Parser::test(SymSet s1, SymSet s2, int no) {
	if (!s1.count(current_token.getSymbol())) {
		// **** error_handler;
		
		s1.insert(s2.begin(), s2.end());
		skip(s2);
	}
}

void Parser::testSemicolon() {

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
	while (current_token.getSymbol() == Symbol::constsy) {
		nextSym();
		constDefinition();
	}

	SymSet varFunDeclBegSym = { Symbol::intsy, Symbol::charsy, Symbol::voidsy };

	// **** test(), skip to varFunDeclBegSym
	// variable definition
	// function
	while (!current_token.isEnd() && varFunDeclBegSym.count(current_token.getSymbol())) {
		if (current_token.getSymbol() == Symbol::voidsy) {
			varDefDone = true;
		}
		type_token = current_token;		// record type of var or func
		nextSym();
		name_token = current_token;	// record name of var or func
		// check if redefine an identifier
		// **** searchTab(current_token);
		nextSym();
		if (current_token.getSymbol() == Symbol::lparent) {
			varDefDone = true;
		}
		if (type_token.getSymbol() == Symbol::voidsy && name_token.getIdentName() == "main") {
			// main function no parameters, no return value;

			break;
		}
		if (varDefDone) {
			if (type_token.getSymbol() == Symbol::voidsy) {
				function();
			}
			else {
				function();
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
	Symbol sy = current_token.getSymbol();
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
		if (current_token.getSymbol() == intcon) {
			return current_token.getConstNum() * sign;
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
		if (current_token.getSymbol() != Symbol::becomes) {
			// error_handler.reportErrorMsg();
		}
		else {
			nextSym();
		}
		
		if (type_token.getSymbol() == Symbol::intsy) {
			int tmpConstInt = signedInt({});	// ?????????????????????? fsys
			// enter symbol table
		}
		else if (type_token.getSymbol() == Symbol::charsy) {
			if (current_token.getSymbol() == charcon) {
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
		sy = current_token.getSymbol();
		if (sy == intcon || sy == charcon) {
			// token is a constant, check type
			if (type_token.getSymbol() == intcon && sy == intcon) {
				int tmpContstInt = current_token.getConstNum();
				// enter symbol table

				nextSym();
			}
			else if (type_token.getSymbol() == charcon && sy == charcon) {
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
	} while (current_token.getSymbol() == Symbol::comma);
	if (current_token.getSymbol() == Symbol::semicolon) {
		nextSym();
	}
	// **** test; // check semicolon, 
	// skip to comma, ident, int, char, void,
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	cout << lc << "行: ";
	cout << "常量定义语句" << endl;
#endif
}


void Parser::varDeclaration() {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	cout << lc << "行: ";
	cout << "变量声明语句" << endl;
#endif // DEBUG_Gramma_analysis
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

		if (current_token.getSymbol() == Symbol::lsquare) {
			// array type
			nextSym();
			Symbol sy = current_token.getSymbol();
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
			if (current_token.getSymbol() == Symbol::intcon) {
				tmpArrayMax = current_token.getConstNum();
				nextSym();
				if (current_token.getSymbol() == Symbol::rsquare) {
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
	} while (current_token.getSymbol() == Symbol::comma);
	// **** test(); 
	// must semicolon end the declar
#ifdef DEBUG_Gramma_analysis
	cout << "This is variable declaration statement!" << endl;
#endif // DEBUG_Gramma_analysis
}

void Parser::compoundstatement() {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	cout << lc << "行: ";
	cout << "复合语句" << endl;
#endif // DEBUG_Gramma_analysis
	// 
	// while sy in statement begin symbol set
	while (current_token.getSymbol() == Symbol::constsy) {
		constDefinition();
	}
	while (current_token.getSymbol() == Symbol::intsy
		|| current_token.getSymbol() == Symbol::charsy) {
		nextSym();
		if (current_token.getSymbol() == Symbol::ident) {
			nextSym();
		}
		else {
			// error: missing ident name
#ifdef DEBUG_Gramma_analysis
			cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
			cout << "出错：缺少标识符" << endl;
#endif // DEBUG_Gramma_analysis
		}
		varDeclaration();
	}
	if (statementBsys.count(current_token.getSymbol())) {
		multiStatement();
	}
}

void Parser::multiStatement() {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	cout << lc << "行: ";
	cout << "语句列" << endl;
#endif // DEBUG_Gramma_analysis
	test({}, {}, 0);
	while (statementBsys.count(current_token.getSymbol())) {
		statement();
	}
}

void Parser::statement() {
	Symbol sy = current_token.getSymbol();
	if (statementBsys.count(sy)) {
		switch (sy) {
		case Symbol::lbrace: {
			multiStatement();
			test({ Symbol::rbrace }, {}, 0);
			if (current_token.getSymbol() == Symbol::rbrace) {
				nextSym();
			}
			break;
		}
		case Symbol::ifsy:		ifStatement({});		break;
		case Symbol::switchsy:	switchStatement();	break;
		case Symbol::whilesy:	whileStatement();	break;
		case Symbol::scanfsy:	read();				break;
		case Symbol::printfsy:	write();			break;
		case Symbol::ident: {
			// search symbol table
			name_token = current_token;
			nextSym();
			// ？？？？？？？？？？？？？？？？？？？？？？需要改进
			if (current_token.getSymbol() == Symbol::lsquare) {
				selector();
			}
			if (current_token.getSymbol() == Symbol::becomes) {
				assignStatement();
			}
			else if (current_token.getSymbol() == Symbol::lparent) {
				call();
			}
			else {
#ifdef DEBUG_Gramma_analysis
				cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
				cout << "出错：标识符后需要等号或小括号！" << endl;
#endif // DEBUG_Gramma_analysis
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

void Parser::selector() {
	if (current_token.getSymbol() == Symbol::lsquare) {
		nextSym();	// read a int con
		test({ Symbol::intcon }, {}, 0);		// ????????????????????????????
		if (current_token.getSymbol() == intcon) {
			nextSym();
		}
		// check right square
		test({ Symbol::rsquare }, {}, 0);	// ???????????????
		if (current_token.getSymbol() == Symbol::rsquare) {
			nextSym();
		}
	}
	// do something
}

void Parser::assignStatement() {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	cout << lc << "行: ";
	cout << "赋值语句" << endl;
#endif // DEBUG_Gramma_analysis
	/*		？？？？？？？？？？？？？？？？？？？？？？？？？
	current_token is name
	nextSym();	// read =
	*/
	test({ Symbol::becomes }, {}, 0);
	if (current_token.getSymbol() == Symbol::becomes) {
		nextSym();
		expression();
	}
	// do something
	testSemicolon();
}

void Parser::ifStatement(SymSet fsys) {
	// ？？？？？？？？？？？？？？？？？？？？？？？？？error id
	//SymSet fsys;
	fsys.insert(statementBsys.begin(), statementBsys.end());
	fsys.insert({ Symbol::lbrace });
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	cout << lc << "行: ";
	cout << "if语句" << endl;
#endif // DEBUG_Gramma_analysis
	nextSym();
	test({ Symbol::lparent }, statementBsys, 0);
	if (current_token.getSymbol() == Symbol::lparent) {
		nextSym();
		condition();
		if (current_token.getSymbol() == Symbol::rparent) {
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

		if (current_token.getSymbol() == lbrace) {
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
	if (current_token.getSymbol() == lparent) {
		nextSym();
		expression();
		if (current_token.getSymbol() == rparent) {
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
	if (current_token.getSymbol() == Symbol::colon) {
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

void Parser::whileStatement() {
	nextSym();
	test({ Symbol::lparent }, {}, 0);
	if (current_token.getSymbol() == Symbol::lparent) {
		nextSym();
		condition();
		test({ Symbol::rparent }, {}, 0);
		if (current_token.getSymbol() == Symbol::rparent) {
			nextSym();
			statement();
		}
	}
}

void Parser::paraList() {
	/* begin with left parent, end with right parent*/
	// 以什么字符开头？？？？？？？？？？？？？？？？？？暂定进入函数前读入了一个括号
	do {
		nextSym();
		if (current_token.getSymbol() == Symbol::rparent) {
			// done
			break;
		}
		else if (current_token.getSymbol() == Symbol::intsy || current_token.getSymbol() == Symbol::charsy) {
			type_token = current_token;
			nextSym();
			if (current_token.getSymbol() == Symbol::ident) {
				name_token = current_token;
				nextSym();
			}
		}
		else {
			// error: missing type define
		}
	} while (current_token.getSymbol() == Symbol::comma);
	/*
	nextSym();
	while (current_token.getSymbol() == Symbol::intsy
		|| current_token.getSymbol() == Symbol::charsy) {
		type_token = current_token;
		nextSym();
		if (current_token.getSymbol() == Symbol::ident) {
			name_token = current_token;
			nextSym();
			if (current_token.getSymbol() == Symbol::comma) {
				nextSym();
			}
		}


	}
	*/
}

void Parser::function() {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	cout << lc << "行: ";
	cout << "function 语句" << endl;
#endif // DEBUG_Gramma_analysis
	// 以什么字符开头？？？？？？？？？？？？？？？？？？？？？？？？？？
	
	paraList();
	test({ Symbol::rparent }, {}, 0);
	if (current_token.getSymbol() == Symbol::rparent) {
		nextSym();
	}
	test({ Symbol::lbrace }, {}, 0);
	if (current_token.getSymbol() == Symbol::lbrace) {
		nextSym();
		compoundstatement();
	}
	test({ Symbol::rbrace }, {}, 0);
	if (current_token.getSymbol() == Symbol::rbrace) {
		nextSym();
	}
}

void Parser::call() {
	/*
	begin with left parent, end with semicolon.
	*/
	test({ lparent }, {}, 0);
	if (current_token.getSymbol() == lparent) {
		nextSym();
		do {
			expression();
		} while (current_token.getSymbol() == Symbol::comma);
		test({ rparent }, {}, 0);
		if (current_token.getSymbol() == Symbol::rparent) {
			nextSym();
		}
		testSemicolon();
	}
}

void Parser::read() {
	/* begin with left parent, end with semicolon*/
	test({ lparent }, {}, 0);
	if (current_token.getSymbol() == Symbol::lparent) {
		do {
			nextSym();
			if (current_token.getSymbol() == Symbol::rparent) {
				// done
				break;
			}
			else if (current_token.getSymbol() == Symbol::ident) {
				// record
				nextSym();
			}
			else {
				// error:？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？
				cout << "缺少标识符" << endl;
			}
		} while (current_token.getSymbol() == Symbol::comma);
		test({ Symbol::rparent }, {}, 0);
		if (current_token.getSymbol() == Symbol::rparent) {
			nextSym();
		}
		testSemicolon();
	}
}

void Parser::write() {
	/* begin with left parent '(', end with semicolon*/
	bool wHasExp = false;
	bool wHasStr = false;
	bool wHasComma = false;
	test({ lparent }, {}, 0);

	if (current_token.getSymbol() == Symbol::lparent) {
		nextSym();
		if (current_token.getSymbol() == stringcon) {
			// stab
			wHasStr = true;
			nextSym();
		}
		if (current_token.getSymbol() == Symbol::comma) {
			wHasComma = true;
			nextSym();

		}
		if (expBsys.count(current_token.getSymbol())) {
			wHasExp = true;
			expression();
		}
		if (wHasComma) {
			if (!wHasStr) {
#ifdef DEBUG_Gramma_analysis
				cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
				cout << "出错：缺少字符串" << endl;
#endif // DEBUG_Gramma_analysis
			}
			if (!wHasExp) {
#ifdef DEBUG_Gramma_analysis
				cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
				cout << "出错：缺少表达式" << endl;
#endif // DEBUG_Gramma_analysis
			}
		}
		else {
			if (wHasStr && wHasExp) {
#ifdef DEBUG_Gramma_analysis
				cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
				cout << "出错：缺少逗号" << endl;
#endif // DEBUG_Gramma_analysis
			}
		}

		do {
			nextSym();
			if (current_token.getSymbol() == Symbol::rparent) {
				// done
				break;
			}
			else if (current_token.getSymbol() == Symbol::ident) {
				// record
				nextSym();
			}
			else {
				// error:？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？
				cout << "缺少标识符" << endl;
			}
		} while (current_token.getSymbol() == Symbol::comma);
		test({ Symbol::rparent }, {}, 0);
		if (current_token.getSymbol() == Symbol::rparent) {
			nextSym();
		}
		testSemicolon();
	}
}

void Parser::condition() {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	cout << lc << "行: ";
	cout << "condition 语句" << endl;
#endif // DEBUG_Gramma_analysis
	expression();
	if (logicalOperatorSys.count(current_token.getSymbol())) {
		nextSym();
		expression();
	}
	// need record the result
}

void Parser::expression() {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	cout << lc << "行: ";
	cout << "expression 语句" << endl;
#endif // DEBUG_Gramma_analysis
	// need an item record result of an expression
	SymSet itemFsys = {};
	if (current_token.getSymbol() == Symbol::plus
		|| current_token.getSymbol() == Symbol::minus) {
		// terms are connected with + or -
		// record the operator
		nextSym();
		item(itemFsys);
	}
	while (current_token.getSymbol() == Symbol::plus
		|| current_token.getSymbol() == Symbol::minus) {
		nextSym();
		item(itemFsys);
	}

}

void Parser::item(SymSet fsys) {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	cout << lc << "行: ";
	cout << "item 语句" << endl;
#endif // DEBUG_Gramma_analysis

	SymSet tmpFacFsys = fsys;
	tmpFacFsys.insert({Symbol::times, Symbol::slash});
	factor(tmpFacFsys);
	while (current_token.getSymbol() == Symbol::times
		|| current_token.getSymbol() == Symbol::slash) {
		// factor is connected with * or /
		// need to record the operator 
		nextSym();
		factor(tmpFacFsys);
	}
}

void Parser::factor(SymSet fsys) {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	cout << lc << "行: ";
	cout << "factor 语句" << endl;
#endif // DEBUG_Gramma_analysis
	test(factorBsys, fsys, 0);
	if (factorBsys.count(current_token.getSymbol())) {
		switch (current_token.getSymbol()) {
		case Symbol::ident: {
			// search symbol table
			nextSym();
			switch (current_token.getSymbol()) {
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

