#include "pch.h"
#include "Parser.h"


Parser::Parser(Lexer *lexer, Error& error_handler, MidCodeGen& codeGen)
:error_handler(error_handler), codeGen(codeGen) {
	this->lexer = lexer;
}

Parser::~Parser()
{
}

void Parser::nextSym() {
	current_token = lexer->getSym();
}

void Parser::skip(SymSet fsys) {
#ifdef DEBUG_Gramma_analysis
	cout << "skip---------------------------" << endl;
#endif
	while (!fsys.count(current_token.getSymbol())) {
#ifdef DEBUG_Gramma_analysis

		cout << "\t" << current_token.getlc() << "行: \""
			<< current_token.getIdentName() << "\"" << endl;
#endif
		nextSym();

	}
}

void Parser::test(SymSet s1, SymSet s2, int no) {
	if (!s1.count(current_token.getSymbol())) {
		// **** error_handler;
		
		s1.insert(s2.begin(), s2.end());
		s1.insert(Symbol::eofsy);
		skip(s1);
	}
}

void Parser::testSemicolon() {
	if (current_token.getSymbol() == Symbol::semicolon) {
		nextSym();
	}
	else {
		// error
		skip({eofsy});	// ？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？
	}

}

void Parser::enter(const Token& t, ObjectiveType obj, bool isGlobal) {

}


void Parser::parse() {
	// parse the whole source file
	bool varDecDone = false;
	bool isMain = false;
	// const definition, end with?????????????????????????????????????????????????????????????????????????????????????????
	nextSym();
	test({ voidsy, intsy, charsy, constsy }, {}, 1);
	while (current_token.getSymbol() == constsy) {
		constDefinition();
	}

	SymSet varFunDecBegSym = { intsy, charsy, voidsy };

	test(varFunDecBegSym, {}, 2);
	while (varFunDecBegSym.count(current_token.getSymbol())) {
		if (current_token.getSymbol() == Symbol::voidsy) {
			varDecDone = true;
		}
		type_token = current_token;	// record type of var or func
		nextSym();
		name_token = current_token;	// record name of var or func
		test({ ident }, {}, 0);
		// **** searchTab(current_token);
		nextSym();
		if (current_token.getSymbol() == Symbol::lparent) {
			varDecDone = true;
		}

		if (name_token.getIdentName() == "main") {
			if (type_token.getSymbol() == Symbol::voidsy && current_token.getSymbol() == Symbol::lparent) {
				// main function no parameters, no return value;
				nextSym();
				test({ rparent }, { lbrace }, 4);
				if (current_token.getSymbol() == rparent) {
					nextSym();
				}
				test({ lbrace }, compoundBsys, 5);
				if (current_token.getSymbol() == lbrace) {
					hasRet = false;
					test(compoundBsys, {}, 0);
					if (compoundBsys.count(current_token.getSymbol())) {
						nextSym();
					}
					compoundstatement();
				}
			} 
			else {
				error_handler.reportErrorMsg(name_token, 3);
				// error:
			}
			break;
		}
		if (varDecDone) {
			if (type_token.getSymbol() == Symbol::voidsy) {
				hasRet = false;
				function();
			}
			else {
				hasRet = true;
				function();
			}
		}
		else {
			varDeclaration();
		}
	}
	// change the error no
	test({ Symbol::eofsy }, {}, 6);
}
int Parser::signedInt(SymSet fsys) {
	/*
	begin with +, - or number, 
	end with something after the signed integer
	*/
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
			int tmpR = current_token.getConstNum() * sign;
			nextSym();
			return tmpR;
		}
		else {
			// error

			return 0;
		}
	}
}
void Parser::constDefinition() {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "常量定义语句" << endl;
#endif
	nextSym();
	test({intsy, charsy}, {ident}, 0);
	type_token = current_token;
	// error if not char or int
	do {
		nextSym();
		test({ ident }, { comma }, 0);
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
				nextSym();
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
	testSemicolon();
	// skip to comma, ident, int, char, void,
	if (current_token.getSymbol() == Symbol::semicolon) {
		nextSym();
	}
}


void Parser::varDeclaration() {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "变量声明语句" << endl;
#endif // DEBUG_Gramma_analysis
	/* before call this function, we use type_token to record 
	 the type of var and we use name_token to record var's name.
	 And the current_token is not '(', it means not function 
	 declaration. If it is ',', we should declare multiple var.
	 */
	 // check the symbol table, error if redefine var
	bool firstLoop = true;
	do {
		if (!firstLoop) {
			nextSym(); // read an ident
		}
		if (current_token.getSymbol() == Symbol::lsquare) {
			// array type
			nextSym();
			Symbol sy = current_token.getSymbol();
			int tmpArrayMax = 0;
			if (sy == Symbol::plus || sy == Symbol::minus) {
				// error
#ifdef DEBUG_Gramma_analysis
				std::cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
				std::cout << "错误：数组索引必须是无符号整数" << endl;
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
					std::cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
					std::cout << "错误：缺少右中括号" << endl;
#endif // DEBUG_Gramma_analysis
				}
			}
			else {
#ifdef DEBUG_Gramma_analysis
				std::cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
				std::cout << "错误：缺少无符号整数常量" << endl;
#endif // DEBUG_Gramma_analysis
				// skip();
			}
		}
		else {
			// comman variable, enter symbol table
			if (!firstLoop) {
				nextSym();
			}
		}
		firstLoop = false;
	} while (current_token.getSymbol() == Symbol::comma);
	testSemicolon();
}

void Parser::compoundstatement() {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "复合语句" << endl;
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
			std::cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
			std::cout << "出错：缺少标识符" << endl;
#endif // DEBUG_Gramma_analysis
		}
		varDeclaration();
	}
	if (statementBsys.count(current_token.getSymbol())) {
		multiStatement();
	}
}

void Parser::multiStatement() {
	if (current_token.getSymbol() == rbrace) {
#ifdef DEBUG_Gramma_analysis
		int lc = current_token.getlc();
		std::cout << lc << "行: ";
		std::cout << "空语句列" << endl;
#endif // DEBUG_Gramma_analysis
		return;
	}
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "语句列" << endl;
#endif // DEBUG_Gramma_analysis
	test({statementBsys}, {eofsy}, 0);
	while (statementBsys.count(current_token.getSymbol())) {
		statement();
	}
}

void Parser::statement() {
	Symbol sy = current_token.getSymbol();
	if (statementBsys.count(sy)) {
		switch (sy) {
		case Symbol::lbrace: {
			nextSym();
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
		case Symbol::printfsy:	write();				break;
		case Symbol::returnsy:	returnStatement();	break;
		case Symbol::semicolon: {
#ifdef DEBUG_Gramma_analysis
			int lc = current_token.getlc();
			std::cout << lc << "行: ";
			std::cout << "空语句" << endl;
#endif // DEBUG_Gramma_analysis
			nextSym();
			break;
		}
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
				testSemicolon();
			}
			else {
#ifdef DEBUG_Gramma_analysis
				std::cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
				std::cout << "出错：标识符后需要等号或小括号！" << endl;
#endif // DEBUG_Gramma_analysis
			}
			break;
		}
		default: {
#ifdef DEBUG_Gramma_analysis
			std::cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
			std::cout << "出错：不是语句开头符号!" << endl;
#endif // DEBUG_Gramma_analysis
		}
		}
	}
}

void Parser::selector() {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "selector 语句" << endl;
#endif // DEBUG_Gramma_analysis
	if (current_token.getSymbol() == Symbol::lsquare) {
		nextSym();	// read a int con
		test(expBsys, {}, 0);
		if (expBsys.count(current_token.getSymbol())) {
			expression();
		}
		test({ Symbol::rsquare }, {}, 0);	// ???????????????
		if (current_token.getSymbol() == Symbol::rsquare) {
			nextSym();
		}
	}
	// do something
}

void Parser::returnStatement() {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "return 语句" << endl;
#endif // DEBUG_Gramma_analysis
	nextSym();
	test(expBsys, {}, 0);
	if (expBsys.count(current_token.getSymbol())) {
		expression();
		testSemicolon();
	}
}

void Parser::assignStatement() {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "赋值语句" << endl;
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
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "if 语句" << endl;
#endif // DEBUG_Gramma_analysis
	// ？？？？？？？？？？？？？？？？？？？？？？？？？error id
	//SymSet fsys;
	fsys.insert(statementBsys.begin(), statementBsys.end());
	fsys.insert({ Symbol::lbrace });
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
			std::cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
			std::cout << "出错：不是语句开头符号!" << endl;
#endif // DEBUG_Gramma_analysis
		}
		test(statementBsys, {}, 0);
		if (statementBsys.count(current_token.getSymbol())) {
			statement();
		}
	}
}

void Parser::switchStatement() {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "switch语句" << endl;
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
			std::cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
			std::cout << "出错：缺少右括号" << endl;
#endif // DEBUG_Gramma_analysis
		}
	// 	SymSet tmp = statementBsys;
		test({ lbrace }, statementBsys, 0);
		if (current_token.getSymbol() == lbrace) {
			nextSym();
			while (current_token.getSymbol() == casesy) {
				caseStatement();
			}
			test({ defaultsy }, statementBsys, 0);
			if (current_token.getSymbol() == defaultsy) {
				defaultStatement();
			}
			else {
				// error
			}
		}
		test({ rbrace }, statementBsys, 0);
		if (current_token.getSymbol() == rbrace) {
			nextSym();
		}
	}
}

void Parser::caseStatement() {
	/*	begin with case, 
	end with some word after statement	*/
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "case 语句" << endl;
#endif // DEBUG_Gramma_analysis
	nextSym();
	// test expression ??
	expression();
	if (current_token.getSymbol() == Symbol::colon) {
		nextSym();
	}
	else {
#ifdef DEBUG_Gramma_analysis
			std::cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
			std::cout << "出错：缺少冒号" << endl;
#endif // DEBUG_Gramma_analysis
	}
	// test colon
	statement();
}

void Parser::defaultStatement() {
	/*	begin with default, 
	end with some word after statement	*/
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "default 语句" << endl;
#endif // DEBUG_Gramma_analysis
	nextSym();
	if (current_token.getSymbol() == Symbol::colon) {
		nextSym();
	}
	else {
#ifdef DEBUG_Gramma_analysis
			std::cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
			std::cout << "出错：缺少冒号" << endl;
#endif // DEBUG_Gramma_analysis
	}
	// test colon
	statement();
}

void Parser::whileStatement() {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "while 语句" << endl;
#endif // DEBUG_Gramma_analysis
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
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "paraList 参数列表语句" << endl;
#endif // DEBUG_Gramma_analysis
	/* begin with left parent, end with right parent*/
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
	std::cout << lc << "行: ";
	if (hasRet)
		std::cout << "有返回值函数定义" << endl;
	else 
		std::cout << "无返回值函数定义" << endl;
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
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "call 语句" << endl;
#endif // DEBUG_Gramma_analysis
	/*
	begin with left parent, end with semicolon.
	*/
	test({ lparent }, {}, 0);
	if (current_token.getSymbol() == lparent) {
		nextSym();
		if (expBsys.count(current_token.getSymbol())) {
			expression();
		}
		while (current_token.getSymbol() == Symbol::comma) {
			nextSym();
			expression();
		}
		/*
		do {
			expression();
		} while (current_token.getSymbol() == Symbol::comma);
		*/
		test({ rparent }, {}, 0);
		if (current_token.getSymbol() == Symbol::rparent) {
			nextSym();
		}
	}
}

void Parser::read() {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "scanf 语句" << endl;
#endif // DEBUG_Gramma_analysis
	/* begin with left parent, end with semicolon*/
	nextSym();
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
				std::cout << "缺少标识符" << endl;
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
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "printf 语句" << endl;
#endif // DEBUG_Gramma_analysis
	/* begin with printf, end with semicolon*/
	bool wHasExp = false;
	bool wHasStr = false;
	bool wHasComma = false;
	nextSym();
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
				std::cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
				std::cout << "出错：缺少字符串" << endl;
#endif // DEBUG_Gramma_analysis
			}
			if (!wHasExp) {
#ifdef DEBUG_Gramma_analysis
				std::cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
				std::cout << "出错：缺少表达式" << endl;
#endif // DEBUG_Gramma_analysis
			}
		}
		else {
			if (wHasStr && wHasExp) {
#ifdef DEBUG_Gramma_analysis
				std::cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
				std::cout << "出错：缺少逗号" << endl;
#endif // DEBUG_Gramma_analysis
			}
		}
		test({ rparent }, {}, 0);
		if (current_token.getSymbol() == Symbol::rparent) {
			nextSym();
		}
		testSemicolon();
	}
}

void Parser::condition() {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "condition 语句" << endl;
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
	std::cout << lc << "行: ";
	std::cout << "expression 语句" << endl;
#endif // DEBUG_Gramma_analysis
	// need an item record result of an expression
	SymSet itemFsys = {};
	if (expBsys.count(current_token.getSymbol())) {
		if (current_token.getSymbol() == Symbol::plus
			|| current_token.getSymbol() == Symbol::minus) {
			// terms are connected with + or -
			// record the operator
			nextSym();
		}
		item(itemFsys);
		while (current_token.getSymbol() == Symbol::plus
			|| current_token.getSymbol() == Symbol::minus) {
			nextSym();
			item(itemFsys);
		}
	}
	// invalid expressiion
}

void Parser::item(SymSet fsys) {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "item 语句" << endl;
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
	std::cout << lc << "行: ";
	std::cout << "factor 语句" << endl;
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
			if (current_token.getSymbol() == rparent) {
				nextSym();
			}
			break;
		}

		}
	}
}

