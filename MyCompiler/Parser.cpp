#include "pch.h"
#include "Parser.h"


Parser::Parser(Lexer *lexer, Error& error_handler, MidCodeGen& codeGen, SymbolTable& symTab)
:error_handler(error_handler), codeGen(codeGen), symTab(symTab) {
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
		cout << "\t" << current_token.getlc() << "行:  symbol:"
			 << Enum2Str(current_token.getSymbol()) << " \"" 
			 << current_token.getIdentName() << "\" " << endl;
#endif
		nextSym();

	}
}

void Parser::test(SymSet s1, SymSet s2, int no) {
	if (!s1.count(current_token.getSymbol())) {
		//  error_handler;
		error_handler.reportErrorMsg(current_token, no);
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
		SymSet fsys = compoundBsys;
		fsys.insert(eofsy);
		fsys.insert(voidsy);
		fsys.insert(rbrace);
		skip(fsys);	
	}

}

void Parser::parse() {
	// parse the whole source file
	SymbolItem* mainLabel = codeGen.genLabel("main");
	codeGen.emit1(Operator::jmp, mainLabel);
	bool varDecDone = false;
	bool isMain = false;
	nextSym();
	// check type 
	test({ voidsy, intsy, charsy, constsy }, statementBsys, 1);
	while (current_token.getSymbol() == constsy) {
		constDefinition();
	}

	SymSet varFunDecBegSym = { intsy, charsy, voidsy };
	test(varFunDecBegSym, {}, 2);
	while (varFunDecBegSym.count(current_token.getSymbol())) {
		if (current_token.getSymbol() == Symbol::voidsy) {
			varDecDone = true;
		}
		SymSet fsys = compoundBsys;
		SymSet tmpSet = { voidsy, intsy, charsy };
		type_token = current_token;	// record type of var or func
		test(tmpSet, compoundBsys, 1);
		if (tmpSet.count(current_token.getSymbol())) {
			nextSym();
		}
		name_token = current_token;	// record name of var or func
		test({ ident }, compoundBsys, 7);
		if (current_token.getSymbol() == ident) {
			nextSym();
		}
		if (current_token.getSymbol() == Symbol::lparent) {
			varDecDone = true;
		}
		// main function
		if (name_token.getIdentName() == "main") {
			codeGen.setLabel(mainLabel);
			symTab.enterFunc(name_token, Type::voidtp, mainLabel);
			if (type_token.getSymbol() == Symbol::voidsy && current_token.getSymbol() == Symbol::lparent) {
				isMain = true;
				// main function no parameters, no return value;
				nextSym();
				test({ rparent }, compoundBsys, 4);
				if (current_token.getSymbol() == rparent) {
					nextSym();
				}
				test({ lbrace }, compoundBsys, 5);
				if (current_token.getSymbol() == lbrace) {
					hasRet = false;
					test(compoundBsys, {}, 0);
					if (compoundBsys.count(current_token.getSymbol())) {
						nextSym();
				// set start pc, 
						compoundstatement();
					}
					test({ rbrace }, { eofsy }, 17);
					if (current_token.getSymbol() == rbrace) {
						nextSym();
					}
					symTab.funDone();
				}
			}
			else {
				error_handler.reportErrorMsg(name_token, 3);
				// error:
			}
			break;
		}
		// function 
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
		// variable decleration
		else {
			varDeclaration();
		}
	}
	if (isMain == false) {
		// missing main function
		error_handler.reportErrorMsg(current_token, 16);
	}
	test({ Symbol::eofsy }, {}, 6);
}

Type Parser::tokenType() {
	Symbol tmpSy = type_token.getSymbol();
	Type typ = tmpSy == Symbol::intsy ? Type::inttp :
			   tmpSy == Symbol::charsy ? Type::chartp :
			   tmpSy == Symbol::voidsy ? Type::voidtp : Type::notp;
	return typ;
}

int Parser::signedInt(SymSet fsys) {
	/*
	begin with +, - or number, 
	end with something after the signed integer
	*/
	SymSet tmp = { Symbol::minus, Symbol::plus, Symbol::intcon };
	Symbol sy = current_token.getSymbol();
	int sign = 1;
	test(tmp, fsys, 11);
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
			error_handler.reportErrorMsg(current_token, 8);
			fsys.insert(tmp.begin(), tmp.end());
			skip(fsys);
			return 0;
		}
	}
}

void Parser::constDefinition() {
	/* begin with const, end with token after semicolon, 
	type check, ident check*/
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "常量定义语句" << endl;
#endif
	nextSym();
	// type check
	test({intsy, charsy}, compoundBsys, 1);	
	if (current_token.getSymbol() == intsy || current_token.getSymbol() == charsy) {
		type_token = current_token;
		// error if not char or int
		do	{
			// check identifier name
			nextSym();
			SymSet fsys = compoundBsys;
			fsys.insert({ comma, semicolon });
			test({ ident }, fsys, 7);
			if (current_token.getSymbol() == ident) {
				name_token = current_token;
				nextSym();
			}
			// check assign symbol =
			if (current_token.getSymbol() != Symbol::becomes) {
				error_handler.reportErrorMsg(current_token, 10);
				SymSet fsys = {
					becomes, ident, comma, semicolon,
					Symbol::plus, Symbol::minus, intcon, charcon };
				skip(fsys);
			}
			else {
				nextSym();
			}
			// check const value
			if (type_token.getSymbol() == Symbol::intsy) {
				// 在signedInt中报错
				int tmpConstInt = signedInt({comma, semicolon, ident, intsy, charsy, voidsy, lbrace, eofsy});
				symTab.enterConst(name_token, Type::inttp, tmpConstInt);
				// enter symbol table
			}
			else if (type_token.getSymbol() == Symbol::charsy) {
				if (current_token.getSymbol() == charcon) {
					// enter symbol table 
					symTab.enterConst(name_token, Type::chartp, current_token.getIdentName()[0]);
					nextSym();
				}
				else {
					// error: type not coherent
					error_handler.reportErrorMsg(current_token, 12);
					skip({ comma, ident, semicolon, intsy, charsy, voidsy, lparent, lbrace });
				}
			}

		} while (current_token.getSymbol() == Symbol::comma);
		testSemicolon();
	}
}

void Parser::varDeclaration() {
	/* before call this function, we use type_token to record 
	 the type of var and we use name_token to record var's name.
	 And the current_token is not '(', it means not function 
	 declaration. If it is ',', we should declare multiple var.
	ident check, redefine check
	 */
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "变量声明语句" << endl;
#endif // DEBUG_Gramma_analysis
	bool firstLoop = true;
	do {
		if (!firstLoop) {
			// read an ident name
			nextSym();
			// check identifier name
			SymSet fsys = compoundBsys;
			fsys.insert({ comma, semicolon });
			test({ ident }, fsys, 7);
			// read a comma or lsquare or semicolon
			if (current_token.getSymbol() == ident) {
				name_token = current_token;
				nextSym();
			}
		}
		firstLoop = false;
		// var type
		Type tmp = tokenType();
		if (tmp == notp) {
			// illegal type
			error_handler.reportErrorMsg(type_token, 2);
		}
		// parse var
		if (current_token.getSymbol() == Symbol::lsquare) {
			// array type
			nextSym();
			Symbol sy = current_token.getSymbol();
			int tmpArrayMax = 0;
			if (sy == Symbol::plus || sy == Symbol::minus) {
				// error
				error_handler.reportErrorMsg(current_token, 13);
				nextSym();
			}
			// read a num
			SymSet fsys = compoundBsys;
			fsys.insert({ rbrace, comma });
			test({ intcon }, fsys, 8);
			if (current_token.getSymbol() == Symbol::intcon) {
				// enter table
				tmpArrayMax = current_token.getConstNum();
				symTab.enterVar(name_token, tmp, tmpArrayMax);
				nextSym();
				test({ rsquare }, fsys, 14);
				if (current_token.getSymbol() == Symbol::rsquare) {
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
			}
		}
		else {
			// comman variable, enter symbol table
			symTab.enterVar(name_token, tmp, 0);
		}
	} while (current_token.getSymbol() == Symbol::comma);
	testSemicolon();
}

void Parser::paraList() {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "paraList 参数列表语句" << endl;
#endif // DEBUG_Gramma_analysis
	/* begin with left parent, end with right parent*/
	SymSet fsys = compoundBsys;
	fsys.insert({ rparent, lbrace, ident });
	do {
		nextSym();
		// right parent, finish loop
		if (current_token.getSymbol() == Symbol::rparent) {
			break;
		}
		// type 
		else if (current_token.getSymbol() == Symbol::intsy || current_token.getSymbol() == Symbol::charsy) {
			type_token = current_token;
			nextSym();
			test({ ident }, fsys, 7);
			if (current_token.getSymbol() == Symbol::ident) {
				name_token = current_token;
				symTab.enterPara(name_token, tokenType());
				nextSym();
			}
		}
		else {
			// error: missing type define
			error_handler.reportErrorMsg(current_token, 1);
			skip(fsys);
		}
	} while (current_token.getSymbol() == Symbol::comma);
}

SymbolItem* Parser::function() {
	/* begin with left parent, 
	   end with symbol after right brace
	   void foo(
	   */
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	if (hasRet)
		std::cout << "有返回值函数定义" << endl;
	else 
		std::cout << "无返回值函数定义" << endl;
#endif // DEBUG_Gramma_analysis
	SymbolItem* label = codeGen.genLabel("fun");
	codeGen.setLabel(label);
	codeGen.emit1(Operator::stRa, symTab.getFunItem());
	Type typ = tokenType();
	if (typ == Type::notp) {
		error_handler.reportErrorMsg(type_token, 2);
	}
	symTab.enterFunc(name_token, typ, label);
	paraList();
	test({ Symbol::rparent }, compoundBsys, 20);
	if (current_token.getSymbol() == Symbol::rparent) {
		nextSym();
	}
	test({ Symbol::lbrace }, compoundBsys, 5);
	if (current_token.getSymbol() == Symbol::lbrace) {
		nextSym();
		compoundstatement();
		if (returned == false && hasRet) {
			error_handler.reportErrorMsg(current_token, 42);
		}
		if (returned == false && !hasRet) {
			codeGen.emit1(Operator::returnOp, symTab.getFunItem());
		}
	}
	//?????????????????????????????????????????????????????????????????????????????????
	test({ Symbol::rbrace }, compoundBsys, 17);
	if (current_token.getSymbol() == Symbol::rbrace) {
		nextSym();
	}
	symTab.funDone();
	//?????????????????????????????????????????????????????????????????????????????????
	return NULL;
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
		type_token = current_token;
		nextSym();
		test({ ident }, compoundBsys, 7);
		if (current_token.getSymbol() == Symbol::ident) {
			name_token = current_token;
			nextSym();
			varDeclaration();
		}
		else {
#ifdef DEBUG_Gramma_analysis
			std::cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
			std::cout << "出错：缺少标识符" << endl;
#endif // DEBUG_Gramma_analysis
		}
	}
	if (statementBsys.count(current_token.getSymbol())) {
		multiStatement();
	}
}

void Parser::multiStatement() {
	/* begin with begining symbol of statement*/
	if (current_token.getSymbol() == rbrace) {
		// empty multi statements
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
	test({statementBsys}, {eofsy}, 16);
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
			test({ Symbol::rbrace }, statementBsys, 18);
			if (current_token.getSymbol() == Symbol::rbrace) {
				nextSym();
			}
			break;
		}
		case Symbol::ifsy:		ifStatement({});	returned = false;	break;
		case Symbol::switchsy:	switchStatement();	returned = false;	break;
		case Symbol::whilesy:	whileStatement();	returned = false;	break;
		case Symbol::scanfsy:	read();				returned = false;	break;
		case Symbol::printfsy:	write();			returned = false;	break;
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
			SymbolItem* tmp = symTab.search(name_token);
			if (tmp == NULL) {
				SymSet fsys = statementBsys;
				fsys.insert({ Symbol::eql });
				fsys.insert(expBsys.begin(), expBsys.end());
				skip(fsys);
				break;
			}
			if (tmp->obj == ObjectiveType::functy) {
				SymSet fsys = statementBsys;
				fsys.insert({ rbrace, eofsy });
				fsys.insert(statementBsys.begin(), statementBsys.end());
				test({ lparent }, fsys, 19);
				if (current_token.getSymbol() == lparent) {
					SymSet fsys = statementBsys;
					fsys.insert({ Symbol::semicolon, Symbol::rbrace });
					call(fsys, tmp);
					codeGen.emit1(Operator::call, tmp);
					testSemicolon();
				}
			}
			else if (tmp->obj == ObjectiveType::varty || tmp->obj == ObjectiveType::arrayty){
				SymbolItem* arrIndex = NULL;
				if (tmp->obj == ObjectiveType::arrayty) {
					SymSet fsys = expBsys;
					fsys.insert(statementBsys.begin(), statementBsys.end());
					fsys.insert({ Symbol::rsquare, Symbol::eql });
					test({ lsquare }, fsys, 21);
					if (current_token.getSymbol() == Symbol::lsquare) {
						SymSet fsys = statementBsys;
						fsys.insert({ Symbol::rbrace, Symbol::becomes, Symbol::rsquare});
						arrIndex = selector(fsys);
					}
				}
				test({ becomes }, statementBsys, 10);
				if (current_token.getSymbol() == Symbol::becomes) {
					//////////////////////////////////////////////////////////////
					SymbolItem * assignRet = assignStatement();
					codeGen.emit3(Operator::assignOp, assignRet, arrIndex, tmp);
				}
				else {
#ifdef DEBUG_Gramma_analysis
					std::cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
					std::cout << "出错：标识符后需要等号或小括号！" << endl;
#endif // DEBUG_Gramma_analysis
				}
			}
			else {
				error_handler.reportErrorMsg(name_token, 24);
				skip(statementBsys);
			}
			returned = false;
			break;
		}
		default: break;		// 默认是空白语句
		}
	}
}

SymbolItem* Parser::selector(SymSet fsys) {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "selector 语句" << endl;
#endif // DEBUG_Gramma_analysis
	SymbolItem* r = NULL;
	if (current_token.getSymbol() == Symbol::lsquare) {
		nextSym();	// read a int con
		fsys.insert({ rsquare });
		r = expression(fsys);
		test({ Symbol::rsquare }, expBsys, 14);
		if (current_token.getSymbol() == Symbol::rsquare) {
			nextSym();
		}
	}
	return r;
}

void Parser::returnStatement() {
	/* begin with return, end with the sym after semicolon
	return(expression());	// has return value function
	return;
	quadruples: return r; or return;
	*/
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "return 语句" << endl;
#endif // DEBUG_Gramma_analysis
	SymbolItem* r = NULL;

	SymSet fsys = statementBsys;
	fsys.insert({ rbrace });
	fsys.insert(expBsys.begin(), expBsys.end());
	nextSym();
	if (hasRet) {	// function with return 
		test({ lparent }, {}, 19);
		if (current_token.getSymbol() == Symbol::lparent) {
			nextSym();
		}
		test(expBsys, fsys, 18);
		if (expBsys.count(current_token.getSymbol())) {
			fsys = statementBsys;
			fsys.insert({ Symbol::rbrace, Symbol::semicolon });
			r = expression(fsys);
			test({ rparent }, fsys, 20);
			if (current_token.getSymbol() == rparent) {
				nextSym();
				testSemicolon();
			}
			codeGen.emit2(Operator::returnOp, symTab.getFunItem(), r);
			returned = true;
		}
	}
	else {		// function without return
		testSemicolon();
		codeGen.emit1(Operator::returnOp, symTab.getFunItem());
		returned = true;
	}
}

SymbolItem* Parser::assignStatement() {
	/* begin with '=', end with sym after semicolon*/
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "赋值语句" << endl;
#endif // DEBUG_Gramma_analysis
	SymSet fsys = statementBsys;
	fsys.insert({ Symbol::rbrace, Symbol::semicolon });
	fsys.insert(expBsys.begin(), expBsys.end());
	SymbolItem* r = NULL;
	test({ Symbol::becomes }, fsys, 10);
	if (current_token.getSymbol() == Symbol::becomes) {
		nextSym();
		r = expression(fsys);
	}
	// do something
	testSemicolon();
	if (r == NULL) {
		r = new SymbolItem();
	}
	return r;
}

void Parser::ifStatement(SymSet fsys) {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "if 语句" << endl;
#endif // DEBUG_Gramma_analysis
	SymbolItem* endifLabel = codeGen.genLabel("end_if");
	fsys.insert(statementBsys.begin(), statementBsys.end());
	fsys.insert({ Symbol::lbrace, Symbol::rbrace });
	nextSym();
	test({ Symbol::lparent }, statementBsys, 19);
	if (current_token.getSymbol() == Symbol::lparent) {
		nextSym();
		condition(endifLabel);
		/////////////////////////////////////////////////////////////////
		test({ Symbol::rparent }, fsys, 20);
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
		if (statementBsys.count(current_token.getSymbol())) {
			statement();
		}
		codeGen.setLabel(endifLabel);
	}
}

void Parser::switchStatement() {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "switch语句" << endl;
#endif // DEBUG_Gramma_analysis
	map<int, Type> caseTab;
	SymbolItem* tmpExp = NULL;
	nextSym();
	SymSet fsys = statementBsys;
	fsys.insert(expBsys.begin(), expBsys.end());
	fsys.insert({ Symbol::lbrace, Symbol::rparent });
	test({ Symbol::lparent }, fsys, 19);
	if (current_token.getSymbol() == lparent) {
		nextSym();
		SymSet fsys = statementBsys;
		fsys.insert({ Symbol::rparent, Symbol::lbrace });
		tmpExp = expression(fsys);	// exp value
		/* exp check ?? 
		if (tmpExp == NULL) {
		error_handler.reportErrorMsg(current_token,27);
		SymSet fsys = statementBsys;
		fsys.insert({ Symbol::casesy, Symbol::defaultsy, Symbol::rbrace });
		skip(fsys);
		return;
		}
		*/
		fsys = statementBsys;
		fsys.insert({ Symbol::rbrace, Symbol::casesy });
		test({ Symbol::rparent }, fsys, 20);
		if (current_token.getSymbol() == rparent) {
			nextSym();
		}
		SymbolItem* end_switch = codeGen.genLabel("end_switch");
		test({ lbrace }, fsys, 5);
		if (current_token.getSymbol() == lbrace) {
			nextSym();
			fsys.erase({ Symbol::rparent });
			test({ casesy }, fsys, 25);
			while (current_token.getSymbol() == casesy) {
				caseStatement(tmpExp, end_switch, &caseTab);
			}
			fsys.erase({ Symbol::casesy });
			test({ defaultsy }, fsys, 26);
			if (current_token.getSymbol() == defaultsy) {
				defaultStatement();
			}
		}
		codeGen.setLabel(end_switch);
		test({ rbrace }, statementBsys, 17);
		if (current_token.getSymbol() == rbrace) {
			nextSym();
		}
	}
}

void Parser::caseStatement(SymbolItem* expItem, SymbolItem* end_switch, map<int, Type> *caseTab) {
	/*	begin with case, 
	end with some words after statement
	convert to if, else
	*/
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "case 语句" << endl;
#endif // DEBUG_Gramma_analysis
	nextSym();
	// read a const value
	SymbolItem* constValue = NULL;
	SymbolItem* end_case = codeGen.genLabel("end_case");

	SymSet fsys = statementBsys;
	fsys.insert({ Symbol::casesy, Symbol::colon, Symbol::rbrace });
	Symbol sy = current_token.getSymbol();
	test(conValueBsys, fsys, 40);
	if (conValueBsys.count(sy)) { // check duplication
		Type ty;
		int conVal = 0;
		if (sy == Symbol::charcon) {
			ty = Type::chartp;
			conVal = current_token.getConstChar();
			constValue = codeGen.genCon(ty, conVal);
			nextSym();
		}
		else {
			ty = Type::inttp;
			conVal = this->signedInt(fsys);
			constValue = codeGen.genCon(ty, conVal);
		}
		if (caseTab->count(conVal) && (*caseTab)[conVal] == ty) {
			error_handler.reportErrorMsg(current_token, 41);
		}
		else {
			(*caseTab)[conVal] = ty;
		}
	} 
	else {
		// make sure constValue != NULL
		constValue = new SymbolItem();
	}
	codeGen.emit3(Operator::jne, expItem, constValue, end_case);
	// test colon
	test({ Symbol::colon }, fsys, 38);
	if (current_token.getSymbol() == Symbol::colon) {
		nextSym();
	}
	else {
#ifdef DEBUG_Gramma_analysis
			std::cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
			std::cout << "出错：缺少冒号" << endl;
#endif // DEBUG_Gramma_analysis
	}
	statement();
	codeGen.emit1(Operator::jmp, end_switch);
	codeGen.emit1(Operator::setLabel, end_case);
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
	SymSet fsys = statementBsys;
	fsys.insert({ Symbol::rbrace });
	test({ Symbol::colon }, fsys, 38);
	if (current_token.getSymbol() == Symbol::colon) {
		nextSym();
	}
	else {
#ifdef DEBUG_Gramma_analysis
		std::cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
		std::cout << "出错：缺少冒号" << endl;
#endif // DEBUG_Gramma_analysis
	}
	statement();
}

void Parser::whileStatement() {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "while 语句" << endl;
#endif // DEBUG_Gramma_analysis
	SymbolItem* tmpCondition = NULL;
	SymbolItem* end_while = codeGen.genLabel("end_while");
	SymbolItem* start_while = codeGen.genLabel("start_while");
	
	nextSym();
	test({ Symbol::lparent }, {}, 0);
	if (current_token.getSymbol() == Symbol::lparent) {
		nextSym();
		codeGen.setLabel(start_while);
		condition(end_while);	// jump if condition is false
		test({ Symbol::rparent }, statementBsys, 20);
		if (current_token.getSymbol() == Symbol::rparent) {
			nextSym();
			statement();
		}
		codeGen.emit1(Operator::jmp, start_while);
		codeGen.setLabel(end_while);
	}
}



SymbolItem* Parser::call(SymSet fsys, SymbolItem* funItem) {
	/*	begin with left parent, end with semicolon. */
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "call 语句" << endl;
#endif // DEBUG_Gramma_analysis
	codeGen.emit1(Operator::mark, funItem);	// 标记当前pc，保存现场
	SymbolItem* para = NULL;
	list<Type> paraTyps = funItem->paraList;


	test({ lparent }, fsys, 19);
	if (current_token.getSymbol() == lparent) {
		nextSym();
		auto typ = paraTyps.begin();
		while (typ != paraTyps.end()) {
			// 参数写入数据栈
			Token error_token = current_token;
			para = expression(fsys);
			if (*typ == Type::chartp && para->typ == Type::inttp) {
				error_handler.reportErrorMsg(error_token, 39);
			}
			if (*typ == Type::inttp) {
				para->typ = Type::inttp;
			}
			if (para->typ != Type::notp) {
				codeGen.emit1(Operator::stPara, para);
			}
			if (current_token.getSymbol() == Symbol::comma) {
				nextSym();
			}
			typ++;
			if (current_token.getSymbol() == Symbol::rparent && typ != paraTyps.end()) {
				error_handler.reportErrorMsg(current_token, 36);
				break;
			}
		}
		if (typ == paraTyps.end() && current_token.getSymbol() == Symbol::comma) {
			nextSym();
			error_handler.reportErrorMsg(current_token, 37);
			fsys.insert({ Symbol::rparent });
			skip(fsys);
		}
		test({ Symbol::rparent }, fsys, 20);
		if (current_token.getSymbol() == rparent) {
			nextSym();
		}
	}
	codeGen.emit1(Operator::call, funItem->fun_label);
	if (funItem->hasRet) {
		SymbolItem* returnValue = codeGen.genTemp();
		codeGen.emit1(Operator::stRetVal, returnValue);
		return returnValue;
	}
	else {
		return new SymbolItem();
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
	SymSet fsys = statementBsys;
	fsys.insert({ Symbol::rparent, Symbol::rbrace });
	test({ lparent }, fsys, 19);
	if (current_token.getSymbol() == Symbol::lparent) {
		do {
			nextSym();
			test({ Symbol::ident }, fsys, 7);
			if (current_token.getSymbol() == Symbol::ident) {
				SymbolItem* tmpItem = symTab.search(current_token);
				codeGen.emit1(Operator::read, tmpItem);
				nextSym();
			}
			if (current_token.getSymbol() == Symbol::rparent) {
				break;
			}
		} while (current_token.getSymbol() == Symbol::comma);
		test({ Symbol::rparent }, fsys, 0);
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
	Token str, com, exp;
	nextSym();
	SymSet fsys = statementBsys;
	fsys.insert({ Symbol::rbrace });
	test({ lparent }, fsys, 19);

	if (current_token.getSymbol() == Symbol::lparent) {
		nextSym();
		str = current_token;
		if (current_token.getSymbol() == stringcon) {
			// stab
			wHasStr = true;
			nextSym();
			SymbolItem* r = codeGen.genTemp();
			r->typ = Type::stringtp;
			r->ident_name = str.getString();
			codeGen.emit1(Operator::print, r);
		}
		com = current_token;
		if (current_token.getSymbol() == Symbol::comma) {
			wHasComma = true;
			nextSym();
		}
		exp = current_token;
		if (expBsys.count(current_token.getSymbol())) {
			wHasExp = true;
			SymbolItem* r = expression(fsys);
			codeGen.emit1(Operator::print, r);
		}
		codeGen.emit0(Operator::print);
		if (wHasComma) {
			if (!wHasStr) {
				error_handler.reportErrorMsg(str, 31);
#ifdef DEBUG_Gramma_analysis
				std::cout << str.getlc() << "行， " << current_token.getcc() << "列， ";
				std::cout << "出错：缺少字符串" << endl;
#endif // DEBUG_Gramma_analysis
			}
			if (!wHasExp) {
				error_handler.reportErrorMsg(exp, 33);
#ifdef DEBUG_Gramma_analysis
				std::cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
				std::cout << "出错：缺少表达式" << endl;
#endif // DEBUG_Gramma_analysis
			}
		}
		else {
			if (wHasStr && wHasExp) {
				error_handler.reportErrorMsg(com, 32);
#ifdef DEBUG_Gramma_analysis
				std::cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
				std::cout << "出错：缺少逗号" << endl;
#endif // DEBUG_Gramma_analysis
			}
		}
		test({ rparent }, fsys, 20);
		if (current_token.getSymbol() == Symbol::rparent) {
			nextSym();
		}
		testSemicolon();
	}
}

void Parser::condition(SymbolItem* label) {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "condition 语句" << endl;
#endif // DEBUG_Gramma_analysis
	SymSet fsys = statementBsys;
	fsys.insert({ rparent, rbrace });

	SymbolItem* exp1 = expression(fsys);
	SymbolItem* exp2 = NULL;
	Symbol sy = current_token.getSymbol();
	Token syToken = current_token;
	if (logicalOperatorSys.count(sy)) {
		nextSym();
		exp2 = expression(fsys);
	}
	if (exp2 != NULL) {
		// check type
		set<Type> tpSet = { Type::inttp, Type::chartp };
		if (tpSet.count(exp1->typ) &&  tpSet.count(exp2->typ) && exp1->typ != exp2->typ) {
			error_handler.reportErrorMsg(current_token, 30);
		}
		Operator op = codeGen.symbol2Operator(sy);
		codeGen.emit3(op, exp1, exp2, label);
	}
	else {
		codeGen.emit2(Operator::jez, exp1, label);
	}
}

SymbolItem* Parser::expression(SymSet fsys) {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "expression 语句" << endl;
#endif // DEBUG_Gramma_analysis
	// need an item record result of an expression
	SymbolItem* item1 = NULL;
	SymbolItem* item2 = NULL;
	SymbolItem* r = NULL;
	test(expBsys, fsys, 33);
	SymSet itemFsys = { Symbol::plus, Symbol::minus, Symbol::semicolon };
	if (expBsys.count(current_token.getSymbol())) {
		Symbol sy = current_token.getSymbol();
		if (sy == Symbol::plus || sy == Symbol::minus) {
			nextSym();
		}
		item1 = item(itemFsys);
		if (sy == Symbol::minus) {
			r = codeGen.genTemp();
			codeGen.emit2(Operator::neg, item1, r);
			item1 = r;
			r = NULL;
		}
		sy = current_token.getSymbol();
		while (sy == Symbol::plus || sy == Symbol::minus) {
			nextSym();
			item2 = item(itemFsys);
			r = codeGen.genTemp();
			codeGen.emit3(codeGen.symbol2Operator(sy), item1, item2, r);
			item1 = r;
			item2 = r = NULL;
			sy = current_token.getSymbol();
		}
		return item1;
	}
	else {
		return new SymbolItem();
	}
}

SymbolItem* Parser::item(SymSet fsys) {
	// factor is connected with * or /
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "item 语句" << endl;
#endif // DEBUG_Gramma_analysis
	SymbolItem* r = NULL;
	SymbolItem* factor1 = NULL;
	SymbolItem* factor2 = NULL;
	fsys.insert({Symbol::times, Symbol::slash});
	test(expBsys, fsys, 34);
	if (expBsys.count(current_token.getSymbol())) {
		factor1 = factor(fsys);
		Symbol sy = current_token.getSymbol();
		while (sy == Symbol::times || sy == Symbol::slash) {
			nextSym();
			factor2 = factor(fsys);
			r = codeGen.genTemp();
			codeGen.emit3(codeGen.symbol2Operator(sy), factor1, factor2, r);
			factor1 = r;
			factor2 = r = NULL;
			sy = current_token.getSymbol();
		}
		return factor1;
	}
	return new SymbolItem();
}

SymbolItem* Parser::factor(SymSet fsys) {
	/* begin symbol set:
	plus, minus, intcon, charcon, ident
	*/
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "factor 语句" << endl;
#endif // DEBUG_Gramma_analysis
	
	test(expBsys, fsys, 35);
	if (expBsys.count(current_token.getSymbol())) {
		switch (current_token.getSymbol()) {
		case Symbol::ident: {
			// search symbol table
			name_token = current_token;
			SymbolItem *tmpItem = symTab.search(name_token);
			nextSym();
			if (tmpItem) {
				switch (tmpItem->obj) {
				case ObjectiveType::arrayty: {
					SymbolItem* offset = selector(fsys);
					SymbolItem* r = codeGen.genTemp();
					codeGen.emit3(Operator::assignOp, r, offset, tmpItem);
					return r;
					break;
				}
				case ObjectiveType::functy: {
					if (tmpItem->hasRet == false) {
						error_handler.reportErrorMsg(name_token, 28);
					}
					return call(fsys, tmpItem);
					break;
				}
				case ObjectiveType::varty: {
					// load var
					return tmpItem;
					break;
				}
				case ObjectiveType::constty: {
					// displace
					return tmpItem;
					break;
				}
				default: {
					error_handler.reportErrorMsg(name_token, 29);
				}
				}
			}
			else {
				error_handler.reportErrorMsg(name_token, 22);
			}
			break;
		}
		case Symbol::plus:
		case Symbol::minus:
		case Symbol::intcon: {
			// do something
			SymSet fsys = {};
			int tmp = this->signedInt(fsys);
			SymbolItem* r = codeGen.genCon(Type::inttp, tmp);
			return r;
			break;
		}
		case Symbol::charcon: {
			// do something
			SymbolItem* r = codeGen.genCon(Type::chartp, current_token.getConstChar());
			nextSym();
			return r;
			break;
		}
		case Symbol::lparent: {
			nextSym();
			SymbolItem * r = expression(fsys);
			r->typ = Type::inttp;
			test({ rparent }, fsys, 0);
			if (current_token.getSymbol() == rparent) {
				nextSym();
			}
			return r;
			break;
		}
		}
	}
	return new SymbolItem();
}

